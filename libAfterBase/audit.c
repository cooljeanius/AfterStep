/*
 * Copyright (C) 2001 Andrew Ferguson <andrew@owsla.cjb.net>
 * Copyright (C) 2001 Sasha Vasko <sasha at aftercode.net>
 * Copyright (C) 1999 Ethan Fischer <allanon@crystaltokyo.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "config.h"

#include <string.h>							   /* for memset */
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>

#include "astypes.h"
#include "output.h"
#include "ashash.h"
#include "audit.h"
#include "selfdiag.h"

#ifdef DEBUG_ALLOCS

#undef malloc
#undef safemalloc
#undef safecalloc
#undef calloc
#undef realloc
#undef add_hash_item
#undef free
#undef mystrdup
#undef mystrndup

#ifndef X_DISPLAY_MISSING

#undef XCreatePixmap
#undef XCreateBitmapFromData
#undef XCreatePixmapFromBitmapData
#undef XFreePixmap

#undef XCreateGC
#undef XFreeGC

#undef XCreateImage
#undef XGetImage
#undef XSubImage
#undef XDestroyImage

#undef XGetVisualInfo
#undef XGetWindowProperty
#undef XListProperties
#undef XGetTextProperty
#undef XAllocClassHint
#undef XAllocSizeHints
#undef XQueryTree
#undef XGetWMHints
#undef XGetWMProtocols
#undef XGetWMName
#undef XGetClassHint
#undef XGetAtomName
#undef XStringListToTextProperty
#undef XShapeGetRectangles
#undef XFree

#endif /* #ifndef X_DISPLAY_MISSING */
#endif /* DEBUG_ALLOCS */

#include "mystring.h"
#include "safemalloc.h"

int
as_assert (void *p, const char *fname, int line, const char *call)
{
	if (p == NULL)
	{
		fprintf (stderr, "ASSERT FAILED in %s, line# %d (%s())\n", fname, line, call);
		print_simple_backtrace();
	}
	return (p == NULL);
}

#define MAX_AUDIT_ALLOCS 30000

typedef struct mem
{
	void         *ptr;
	const char	 *fname;
	size_t        length;
	short int     type;
	short int     line;
	char          freed;
}
mem;

enum
{
	C_MEM = 0,
	C_MALLOC = 0x100,
	C_CALLOC = 0x200,
	C_REALLOC = 0x300,
	C_ADD_HASH_ITEM = 0x400,
	C_ADD_HASH_OPTIONAL_ITEM = 0x500,
	C_MYSTRDUP = 0x600,
	C_MYSTRNDUP = 0x700,

	C_PIXMAP = 1,
	C_CREATEPIXMAP = 0x100,
	C_BITMAPFROMDATA = 0x200,
	C_FROMBITMAP = 0x300,

	C_GC = 2,

	C_IMAGE = 3,
	C_GETIMAGE = 0x100,
/*    C_XPMFILE = 0x200, *//* must be same as pixmap version above */
	C_SUBIMAGE = 0x300,

	C_XMEM = 4,
	C_XGETVISUALINFO = 	   	(0x100<<0),
	C_XGETWINDOWPROPERTY = 	(0x100<<1),
    C_XLISTPROPERTIES = 	(0x100<<2),
    C_XGETTEXTPROPERTY = 	(0x100<<3),
    C_XALLOCCLASSHINT = 	(0x100<<4),
    C_XALLOCSIZEHINTS = 	(0x100<<5),
    C_XQUERYTREE = 			(0x100<<6),
    C_XGETWMHINTS = 		(0x100<<7),
    C_XGETWMPROTOCOLS = 	(0x100<<8),
    C_XGETWMNAME = 			(0x100<<9),
    C_XGETCLASSHINT = 		(0x100<<10),
    C_XGETATOMNAME = 		(0x100<<11),
    C_XSTRINGLISTTOTEXTPROPERTY = (0x100<<12),
    C_XSHAPEGETRECTANGLES = (0x100<<13),

	C_LAST_TYPE
};

static ASHashTable *allocs_hash = NULL ;

#define DEALLOC_CACHE_SIZE      128
static mem* deallocated_mem[DEALLOC_CACHE_SIZE+10] ;
static unsigned int deallocated_used = 0 ;

static long allocations = 0;
static long reallocations = 0;
static long deallocations = 0;
static long max_allocations = 0;
static unsigned long max_alloc = 0;
static unsigned long max_x_alloc = 0;
static unsigned long total_alloc = 0;
static unsigned long total_x_alloc = 0;
static unsigned long max_service = 0;
static unsigned long total_service = 0;

static int    service_mode = 0 ;
static int    cleanup_mode = 0 ;

int set_audit_cleanup_mode(int mode)
{ int old = cleanup_mode; cleanup_mode = mode ; return old;}

void          count_alloc (const char *fname, int line, void *ptr, size_t length, int type);
mem          *count_find (const char *fname, int line, void *ptr, int type);
mem          *count_find_and_extract (const char *fname, int line, void *ptr, int type);

void mem_destroy (ASHashableValue value,void *data)
{
	if( data != NULL )
	{
		if( deallocated_used < DEALLOC_CACHE_SIZE )
			deallocated_mem[deallocated_used++] = (mem*)data ;
		else
        {
            if( total_service < sizeof(mem) )
			{
                show_error( "MEMORY AUDIT: it seems that we have too little auditing memory (%lu) while deallocating pointer %p.\n   Called from %s:%d", total_service, ((mem*)data)->ptr, ((mem*)data)->fname, ((mem*)data)->line );
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
            }else
                total_service -= sizeof(mem);
            free( data );
        }
	}
}


void
count_alloc (const char *fname, int line, void *ptr, size_t length, int type)
{
    mem          *m = NULL;
	ASHashResult  res ;
	ASHashData hdata = {0};

    if( service_mode > 0 )
		return ;
	if( allocs_hash == NULL )
	{
		service_mode++ ;
		allocs_hash = create_ashash( 256, pointer_hash_value, NULL, mem_destroy );
		fprintf( stderr, "MEMORY AUDIT: count_alloc() called from %s:%d: allocs hash table created with pointer %p\n", fname, line, allocs_hash );
		service_mode-- ;
	}else if( ptr == allocs_hash ) 
		return;		

	if( get_hash_item( allocs_hash, (ASHashableValue)ptr, &hdata.vptr ) == ASH_Success )
	{
		m = (mem*)hdata.vptr ;
		if( type != (C_MEM|C_ADD_HASH_OPTIONAL_ITEM) )
		{	
			show_error( "Same pointer value 0x%lX is being counted twice!\n  Called from %s:%d - previously allocated in %s:%d", (unsigned long)ptr, fname, line, m->fname, m->line );
			print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
			{	char *segv = NULL ;	*segv = 0 ;  }
#endif
		}else
			return ;
	}else if( deallocated_used > 0 )
    {
        m = deallocated_mem[--deallocated_used];
/*        show_warning( "<mem> reusing deallocation cache  - element %d, pointer %p auditing service memory used (%lu )\n   Called from %s:%d",
                        deallocated_used, m, total_service, fname, line );
 */ }else
    {
		m = safecalloc (1, sizeof (mem));
        if( total_service+sizeof(mem) > AUDIT_SERVICE_MEM_LIMIT )
        {
            show_error( "<mem> too much auditing service memory used (%lu - was %lu)- aborting, please investigate.\n   Called from %s:%d",
                        total_service+sizeof(mem), total_service, fname, line );
            print_simple_backtrace();
			output_unfreed_mem (stderr);
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
            exit(0);
        }
        total_service += sizeof(mem);
        if( total_service > max_service )
            max_service = total_service ;
    }
    m->fname = fname;
	m->line = line;
	m->length = length;
	m->type = type;
	m->ptr = ptr;
	m->freed = 0;

	allocations++;
	if ((type & 0xff) == C_MEM)
	{
		total_alloc += length;
		if (total_alloc > max_alloc)
			max_alloc = total_alloc;
	} else
	{
		total_x_alloc += length;
		if (total_x_alloc > max_x_alloc)
			max_x_alloc = total_x_alloc;
	}
	if (allocations - deallocations > max_allocations)
		max_allocations = allocations - deallocations;

	if( (res = add_hash_item( allocs_hash, (ASHashableValue)ptr, m )) != ASH_Success )
		show_error( "failed to log allocation for pointer 0x%lX - result = %d", ptr, res);
    else
    {
        if( total_service+sizeof(ASHashItem) > AUDIT_SERVICE_MEM_LIMIT )
        {
            show_error( "<add_hash_item> too much auditing service memory used (%lu - was %lu)- aborting, please investigate.\n   Called from %s:%d",
                        total_service+sizeof(ASHashItem), total_service, fname, line );
            print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
            exit(0);
        }
        total_service += sizeof(ASHashItem);
        if( total_service > max_service )
            max_service = total_service ;
    }
}

mem          *
count_find (const char *fname, int line, void *ptr, int type)
{
	ASHashData hdata = {0};

	if( allocs_hash != NULL )
		if( get_hash_item( allocs_hash, (ASHashableValue)ptr, &hdata.vptr) == ASH_Success )
		{
			mem *m = hdata.vptr ;
			if( (m->type & 0xff) == (type & 0xff) )
				return m ;
		}
	return NULL ;
}

mem          *
count_find_and_extract (const char *fname, int line, void *ptr, int type)
{
	mem          *m = NULL;
	if( allocs_hash && ptr )
	{
		ASHashData hd ;
		service_mode++ ;
		if( remove_hash_item (allocs_hash, AS_HASHABLE(ptr), &hd.vptr, False) == ASH_Success )
		{
			m = hd.vptr ;
//			if( allocs_hash->items_num <= 0 )
//				destroy_ashash(&allocs_hash);
			if( (m->type & 0xff) != (type & 0xff) && (m->type & 0xff) != C_IMAGE )
			{
            	show_error( "while deallocating pointer %p discovered that it was allocated with different type\n   Called from %s:%d", ptr, fname, line );
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
			}
			if( total_service < sizeof(ASHashItem) )
			{
            	show_error( "it seems that we have too little auditing memory (%lu) while deallocating pointer %p.\n   Called from %s:%d", total_service, ptr, fname, line );
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
      		}else
		        total_service -= sizeof(ASHashItem);
        }
        service_mode-- ;
	}
	/* fprintf( stderr, "looking for ptr %p, produced result %p at %s:%d", ptr, m, fname, line ); */
	if( m )
	{
		if ((m->type & 0xff) == C_MEM)
			total_alloc -= m->length;
		else
			total_x_alloc -= m->length;
		deallocations++;
	}
	return m;
}

void         *
countmalloc (const char *fname, int line, size_t length)
{
	void         *ptr;
    if( (int)length < 0 )
	{
		fprintf( stderr, "too large malloc of %lu from %s:%d\n", (unsigned long)length, fname, line );
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
	}
	ptr = safemalloc (length);
	count_alloc (fname, line, ptr, length, C_MEM | C_MALLOC);
	return ptr;
}

void         *
countcalloc (const char *fname, int line, size_t nrecords, size_t length)
{
	void         *ptr = safecalloc (nrecords, length);

    if( (int)(length*nrecords) < 0 )
	{
		fprintf( stderr, "too large calloc of %lu from %s:%d\n", (unsigned long)length*nrecords, fname, line );
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
	}
	count_alloc (fname, line, ptr, nrecords * length, C_MEM | C_CALLOC);
	return ptr;
}

void         *
countrealloc (const char *fname, int line, void *ptr, size_t length)
{
	if (ptr != NULL && length == 0)
		countfree (fname, line, ptr);
	if (length == 0)
		return NULL;
	if (ptr != NULL)
	{
		mem          *m = NULL;
		ASHashResult  res ;

		if( allocs_hash != NULL )
		{
			ASHashData hd ;
			service_mode++ ;
			if( remove_hash_item (allocs_hash, AS_HASHABLE(ptr), &hd.vptr, False) == ASH_Success )
			{
				m = hd.vptr ;	  
				if( (m->type & 0xff) != C_MEM )
				{
					show_error( "while deallocating pointer 0x%lX discovered that it was allocated with different type", ptr );
					print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
					m = NULL ;
				}
			}
			service_mode-- ;
		}
		if (m == NULL)
		{
			show_error ("countrealloc:attempt in %s:%d to realloc memory(%p) that was never allocated!\n",
					     fname, line, ptr);
			print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
			return NULL;
		}
		if ((m->type & 0xff) == C_MEM)
		{
			total_alloc -= m->length;
			total_alloc += length;
			if (total_alloc > max_alloc)
				max_alloc = total_alloc;
		} else
		{
			total_x_alloc -= m->length;
			total_x_alloc += length;
			if (total_x_alloc > max_x_alloc)
				max_x_alloc = total_x_alloc;
		}
		m->fname = fname;
		m->line = line;
		m->length = length;
		m->type = C_MEM | C_REALLOC;
		m->ptr = saferealloc (ptr, length);
		m->freed = 0;
		ptr = m->ptr;
		if( (res = add_hash_item( allocs_hash, (ASHashableValue)ptr, m )) != ASH_Success )
		{
			show_error( "failed to log allocation for pointer 0x%lX - result = %d", ptr, res);
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
		}
  		reallocations++;
	} else
		ptr = countmalloc (fname, line, length);

	return ptr;
}

void
countfree (const char *fname, int line, void *ptr)
{
	mem          *m ;

	/* fprintf( stderr, "Service mode = %d, alloca_hash = %p ptr = %p at %s:%d\n", service_mode, allocs_hash, ptr, fname, line );*/
    if( service_mode > 0 || allocs_hash == NULL )
		return ;

	if (ptr == NULL)
	{
		show_error("countfree:attempt to free NULL memory in %s:%d", fname, line);
		print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
		return;
	}

	m = count_find_and_extract (fname, line, ptr, C_MEM);
	if (m == NULL)
	{
		if( cleanup_mode == 0 ) 
		{
			show_error( "countfree:attempt in %s:%d to free memory(%p) that was never allocated!", fname, line, ptr);
			print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
		}
		return;
	}
#if 0
// this is invalid code!!
	if (m1->freed > 0)
	{
		fprintf (stderr, "%s:mem already freed %d time(s)!\n", __FUNCTION__, m1->freed);
		fprintf (stderr, "%s:freed from %s:%d\n", __FUNCTION__, (*m1).fname, (*m1).line);
		fprintf (stderr, "%s:called from %s:%d\n", __FUNCTION__, fname, line);
		print_simple_backtrace();
#ifdef DEBUG_ALLOC_STRICT
{	char *segv = NULL ;	*segv = 0 ;  }
#endif
		/* exit (1); */
	} else
		safefree (m1->ptr);
	m1->freed++;
	m1->fname = fname;
	m1->line = line;
#else
	fprintf( stderr, "%s: freeing %p at %s:%d\n", __FUNCTION__, m->ptr, fname, line );
	safefree (m->ptr);
	mem_destroy( (ASHashableValue)NULL, m );
#endif
}

Bool check_hash_item_reused (ASHashItem *item);

ASHashResult
countadd_hash_item (const char *fname, int line, struct ASHashTable *hash, ASHashableValue value, void *data )
{
	ASHashResult   res ;
	
	service_mode++ ;
	res = add_hash_item(hash, value, data );
	service_mode-- ;

	/* show_debug(  __FILE__, __FUNCTION__, __LINE__, "add_hash_item called from %s:%d with args %p, %lX, %p", fname, line, hash, value, data ); */
	if( hash != allocs_hash ) 
	{
		if( res == ASH_Success )
		{	
			if( check_hash_item_reused (hash->most_recent) )
			{	
				/*show_debug( __FILE__, fname, line, "reused hash item %p", hash->most_recent );*/
				count_alloc (fname, line, hash->most_recent, sizeof(ASHashItem), C_MEM | C_ADD_HASH_OPTIONAL_ITEM );
    		}else 
				count_alloc (fname, line, hash->most_recent, sizeof(ASHashItem), C_MEM | C_ADD_HASH_ITEM);
		}
	}
	return res;
}

char* countadd_mystrdup(const char *fname, int line, const char *a)
{
	char *ptr = mystrdup(a);

    if( a != NULL )
		count_alloc (fname, line, ptr, strlen(ptr)+1, C_MEM | C_MYSTRDUP);
	return ptr;
}

char* countadd_mystrndup(const char *fname, int line, const char *a, int len)
{
	char *ptr = mystrndup(a, len );

    if( a != NULL )
		count_alloc (fname, line, ptr, strlen(ptr)+1, C_MEM | C_MYSTRNDUP);
	return ptr;
}

void
output_unfreed_mem (FILE *stream)
{
	ASHashIterator i;
	long asimage_apply_buffer_sum = 0 ;
	long load_freetype_locale_glyph_sum = 0 ;
	long compress_glyph_pixmap_sum = 0 ;
	

    if( stream == NULL )
        stream = stderr ;
    fprintf (stream, "===============================================================================\n");
    fprintf (stream, "Memory audit: %s\n", ApplicationName);
    fprintf (stream, "\n");
    fprintf (stream, "   Total   allocs: %lu\n", allocations);
    fprintf (stream, "   Total reallocs: %lu\n", reallocations);
    fprintf (stream, "   Total deallocs: %lu\n", deallocations);
    fprintf (stream, "Max allocs at any one time: %lu\n", max_allocations);
    fprintf (stream, "Lost audit memory: %lu\n", total_service - deallocated_used*sizeof(mem));
    fprintf (stream, "      Lost memory: %lu\n", total_alloc);
    fprintf (stream, "    Lost X memory: %lu\n", total_x_alloc);
    fprintf (stream, " Max audit memory: %lu\n", max_service);
    fprintf (stream, "  Max memory used: %lu\n", max_alloc);
    fprintf (stream, "Max X memory used: %lu\n", max_x_alloc);
    fprintf (stream, "\n");
    fprintf (stream, "List of unfreed memory\n");
    fprintf (stream, "----------------------\n");
    fprintf (stream, "allocating function    |line |length |pointer    |type (subtype)\n");
    fprintf (stream, "-----------------------+-----+-------+-----------+--------------\n");
	if( start_hash_iteration( allocs_hash, &i ) )
	do
	{
		register mem *m;
		m = curr_hash_data( &i );
		if( m == NULL )
		{
            fprintf (stream, "hmm, wierd, encoutered NULL pointer while trying to check allocation record for %p!", (void*)((*(i.curr_item))->value));
			continue;
		}else if (m->freed == 0)
		{
			
			if( strcmp( m->fname, "asimage_apply_buffer" ) == 0 ) 
				asimage_apply_buffer_sum += (long)m->length ;
			else if( strcmp( m->fname, "load_freetype_locale_glyph" ) == 0 ) 
				load_freetype_locale_glyph_sum += (long)m->length ;
			else if( strcmp( m->fname, "compress_glyph_pixmap" ) == 0 ) 
				compress_glyph_pixmap_sum += (long)m->length ;

            fprintf (stream, "%23s|%-5d|%-7ld|%p ", m->fname, m->line, (long)m->length, m->ptr);
			switch (m->type & 0xff)
			{
			 case C_MEM:
                 fprintf (stream, "| malloc");
				 switch (m->type & ~0xff)
				 {
				  case C_MALLOC:
                      fprintf (stream, " (malloc)");
					  break;
				  case C_CALLOC:
                      fprintf (stream, " (calloc)");
					  break;
				  case C_REALLOC:
                      fprintf (stream, " (realloc)");
					  break;
				  case C_ADD_HASH_ITEM:
                      fprintf (stream, " (add_hash_item)");
					  break;
				  case C_ADD_HASH_OPTIONAL_ITEM :
                      fprintf (stream, " (add_hash_item - reused)");
					  break;
				  case C_MYSTRDUP:
                      fprintf (stream, " (mystrdup)");
					  break;
				  case C_MYSTRNDUP:
                      fprintf (stream, " (mystrndup)");
					  break;
				 }
				 /* if it seems to be a string, print it */
				 {
					 int           i;
					 const unsigned char *ptr = m->ptr;

					 for (i = 0; i < m->length; i++)
					 {
						 if (ptr[i] == '\0')
							 break;
						 /* don't print strings containing non-space control characters or high ASCII */
						 if ((ptr[i] <= 0x20 && !isspace (ptr[i])) || ptr[i] >= 0x80)
							 i = m->length;
					 }
					 if (i < m->length)
                         fprintf (stream, " '%s'", ptr);
				 }
				 break;
			 case C_PIXMAP:
                 fprintf (stream, "| pixmap");
				 switch (m->type & ~0xff)
				 {
				  case C_CREATEPIXMAP:
                      fprintf (stream, " (XCreatePixmap)");
					  break;
				  case C_BITMAPFROMDATA:
                      fprintf (stream, " (XCreateBitmapFromData)");
					  break;
				  case C_FROMBITMAP:
                      fprintf (stream, " (XCreatePixmapFromBitmapData)");
					  break;
				 }
				 break;
			 case C_GC:
                 fprintf (stream, "| gc (XCreateGC)");
				 break;
			 case C_IMAGE:
                 fprintf (stream, "| image");
				 switch (m->type & ~0xff)
				 {
				  case 0:
                      fprintf (stream, " (XCreateImage)");
					  break;
				  case C_GETIMAGE:
                      fprintf (stream, " (XGetImage)");
					  break;
				  case C_SUBIMAGE:
                      fprintf (stream, " (XSubImage)");
					  break;
				 }
				 break;
			 case C_XMEM:
                 fprintf (stream, "| X mem");
				 switch (m->type & ~0xff)
				 {
				  case C_XGETVISUALINFO:
                      fprintf (stream, " (XGetVisualInfo)");
					  break;
				  case C_XGETWINDOWPROPERTY:
                      fprintf (stream, " (XGetWindowProperty)");
					  break;
                  case C_XLISTPROPERTIES:
                      fprintf (stream, " (XListProperties)");
					  break;
                  case C_XGETTEXTPROPERTY:
                      fprintf (stream, " (XGetTextProperty)");
                      break;
                  case C_XALLOCCLASSHINT :
                      fprintf (stream, " (XAllocClassHint)");
                      break;
                  case C_XALLOCSIZEHINTS :
                      fprintf (stream, " (XAllocSizeHints)");
                      break;
                  case C_XQUERYTREE:
                      fprintf (stream, " (XQueryTree)");
					  break;
				  case C_XGETWMHINTS:
                      fprintf (stream, " (XGetWMHints)");
					  break;
				  case C_XGETWMPROTOCOLS:
                      fprintf (stream, " (XGetWMProtocols)");
					  break;
				  case C_XGETWMNAME:
                      fprintf (stream, " (XGetWMName)");
					  break;
				  case C_XGETCLASSHINT:
                      fprintf (stream, " (XGetClassHint)");
					  break;
				  case C_XGETATOMNAME:
                      fprintf (stream, " (XGetAtomName)");
					  break;
				  case C_XSTRINGLISTTOTEXTPROPERTY:
                      fprintf (stream, " (XStringListToTextProperty)");
					  break;
				 }
				 break;
			}
            fprintf (stream, "\n");
		}
	}while( next_hash_item(&i) );
    fprintf (stream, "===============================================================================\n");
	fprintf( stream, "Total memory allocated in :\n");
	fprintf( stream, "\tasimage_apply_buffer() = %ld\n", asimage_apply_buffer_sum );
	fprintf( stream, "\tload_freetype_locale_glyph() = %ld\n", load_freetype_locale_glyph_sum );
	fprintf( stream, "\tcompress_glyph_pixmap() = %ld\n", compress_glyph_pixmap_sum );
    fprintf (stream, "===============================================================================\n");
}

void
spool_unfreed_mem (char *filename, const char *comments)
{
    FILE *spoolfile = fopen(filename, "w+");
    if( spoolfile )
    {
        fprintf( spoolfile, "%s: Memory Usage Snapshot <%s>", ApplicationName, comments?comments:"no comments\n" );
        output_unfreed_mem( spoolfile );
        fclose( spoolfile );
    }
}

void print_unfreed_mem()
{
    output_unfreed_mem(NULL);
}


void
print_unfreed_mem_stats (const char *file, const char *func, int line, const char *msg)
{
    if( msg )
        fprintf( stderr, "%s:%s:%s:%d: Memory audit %s\n", ApplicationName, file, func, line, msg );
    fprintf( stderr, "%s:%s:%s:%d: Memory audit counts: allocs %lu, reallocs: %lu, deallocs: %lu, max simultaneous %lu\n",
                     ApplicationName, file, func, line, allocations, reallocations, deallocations, max_allocations);
    fprintf( stderr, "%s:%s:%s:%d: Memory audit used memory: private %lu, X %lu, audit %lu, max private %lu, max X %lu, max audit %lu\n",
                     ApplicationName, file, func, line, total_alloc, total_x_alloc, total_service - deallocated_used*sizeof(mem), max_alloc, max_x_alloc, max_service);
}


#if !defined(X_DISPLAY_MISSING) && defined(DEBUG_ALLOCS)
Pixmap
count_xcreatepixmap (const char *fname, int line, Display * display,
					 Drawable drawable, unsigned int width, unsigned int height, unsigned int depth)
{
	Pixmap        pmap = XCreatePixmap (display, drawable, width, height, depth);

	if (pmap == None)
		return None;
	count_alloc (fname, line, (void *)pmap, width * height * depth / 8, C_PIXMAP | C_CREATEPIXMAP);
	return pmap;
}


Pixmap
count_xcreatebitmapfromdata (const char *fname, int line, Display * display,
							 Drawable drawable, char *data, unsigned int width, unsigned int height)
{
	Pixmap        pmap = XCreateBitmapFromData (display, drawable, data, width, height);

	if (pmap == None)
		return None;
	count_alloc (fname, line, (void *)pmap, width * height / 8, C_PIXMAP | C_BITMAPFROMDATA);
	return pmap;
}

Pixmap
count_xcreatepixmapfrombitmapdata (const char *fname, int line,
								   Display * display, Drawable drawable,
								   char *data, unsigned int width,
								   unsigned int height, unsigned long fg, unsigned long bg, unsigned int depth)
{
	Pixmap        pmap = XCreatePixmapFromBitmapData (display, drawable, data, width, height,
													  fg,
													  bg, depth);

	if (pmap == None)
		return None;
	count_alloc (fname, line, (void *)pmap, width * height * depth / 8, C_PIXMAP | C_FROMBITMAP);
	return pmap;
}

int
count_xfreepixmap (const char *fname, int line, Display * display, Pixmap pmap)
{
	mem          *m = count_find_and_extract (fname, line, (void *)pmap, C_PIXMAP);

	if (pmap == None)
	{
		show_error("count_xfreepixmap:attempt to free None pixmap in %s:%d", fname, line);
		return !Success;
	}

	if (m == NULL)
	{
		show_error ("count_xfreepixmap:attempt in %s:%d to free Pixmap(0x%X) that was never created, or already freed!",
				 	 fname, line, (unsigned int)pmap);
		raise( SIGUSR2 );
		XFreePixmap (display, pmap );
		return !Success;
	}
/*	fprintf (stderr,"%s:%s:%d freeing Pixmap(0x%X)\n", __FUNCTION__, fname, line, (unsigned int)pmap);
*/

	XFreePixmap (display, pmap);
	mem_destroy( (ASHashableValue)NULL, m );
	return Success;
}

GC
count_xcreategc (const char *fname, int line, Display * display,
				 Drawable drawable, unsigned int mask, XGCValues * values)
{
	GC            gc = XCreateGC (display, drawable, mask, values);

	if (gc == None)
		return None;
	count_alloc (fname, line, (void *)gc, sizeof (XGCValues), C_GC);
	return gc;
}

int
count_xfreegc (const char *fname, int line, Display * display, GC gc)
{
	mem          *m = count_find_and_extract (fname, line, (void *)gc, C_GC);

	if (gc == None)
	{
		show_error( "count_xfreegc:attempt to free None GC in %s:%d", fname, line);
		return !Success;
	}

	if (m == NULL)
	{
		show_error( "count_xfreegc:attempt in %s:%d to free a GC (%p)that was never created or already destroyed!",
				    fname, line, gc);
		return !Success;
	}

	XFreeGC (display, gc);
	mem_destroy( (ASHashableValue)NULL, m );
	return Success;
}

XImage       *
count_xcreateimage (const char *fname, int line, Display * display,
					Visual * visual, unsigned int depth, int format,
					int offset, char *data, unsigned int width, unsigned int height, int bitmap_pad, int byte_per_line)
{
	XImage       *image = XCreateImage (display, visual, depth, format, offset, data, width,
										height,
										bitmap_pad, byte_per_line);

	if (image == NULL)
		return NULL;
	count_alloc (fname, line, (void *)image, sizeof (*image), C_IMAGE);
	return image;
}

XImage       *
count_xgetimage (const char *fname, int line, Display * display,
				 Drawable drawable, int x, int y, unsigned int width,
				 unsigned int height, unsigned long plane_mask, int format)
{
	XImage       *image = XGetImage (display, drawable, x, y, width, height, plane_mask,

									 format);

	if (image == NULL)
		return NULL;
	count_alloc (fname, line, (void *)image,
				 sizeof (*image) + image->height * image->bytes_per_line, C_IMAGE | C_GETIMAGE);
	return image;
}

XImage       *
count_xsubimage (const char *fname, int line, XImage *img,
				 int x, int y, unsigned int width, unsigned int height )

{
	XImage       *image = (*(img->f.sub_image))(img, x, y, width, height);

	if (image == NULL)
		return NULL;
	count_alloc (fname, line, (void *)image,
				 sizeof (*image) + image->height * image->bytes_per_line, C_IMAGE | C_SUBIMAGE);
	return image;
}

XRectangle *XShapeGetRectangles (Display *dpy,Window window,int kind,int *count,int *ordering);

XRectangle       *
count_xshape_get_rectangles (const char *fname, int line, Display *dpy,
				             Window window, int kind, int *count, int *ordering)

{
	
	XRectangle       *rects = XShapeGetRectangles(dpy, window, kind, count, ordering);

	if (rects == NULL)
		return NULL;
	count_alloc (fname, line, (void *)rects, sizeof(*rects)* (*count), C_XMEM|C_XSHAPEGETRECTANGLES);
	return rects;
}

int
count_xdestroyimage (const char *fname, int line, XImage * image)
{
	mem          *m;
	void         *image_data;
	void         *image_obdata;

	if (image == NULL)
	{
		show_error("count_xdestroyimage:attempt to free NULL XImage in %s:%d", fname, line);
		return BadValue;
	}
	image_data = (void *)(image->data);
	image_obdata = (void *)(image->obdata);

	if ((m = count_find (fname, line, (void *)image, C_IMAGE)) == NULL)
		/* can also be of C_MEM type if we allocated it ourselvs */
		if ((m = count_find (fname, line, (void *)image, C_MEM)) == NULL)
		{
			show_error("count_xdestroyimage:attempt in %s:%d to destroy an XImage that was never created or already destroyed.\n",
					    fname, line);
			return !Success;
		}

	(*image->f.destroy_image) (image);

	if ((m = count_find_and_extract (fname, line, (void *)image, C_IMAGE)) == NULL)
		/* can also be of C_MEM type if we allocated it ourselvs */
		m = count_find_and_extract (fname, line, (void *)image, C_MEM);
	if (m)
		mem_destroy( (ASHashableValue)NULL, m );

	/* find and free the image->data pointer if it is in our list */
	if( image_data )
		if ((m = count_find_and_extract (fname, line, image_data, C_MEM)) != NULL)
			mem_destroy( (ASHashableValue)NULL, m );

	/* find and free the image->obdata pointer if it is in our list */
	if( image_obdata )
		if ((m = count_find_and_extract (fname, line, image_obdata, C_MEM)) != NULL)
			mem_destroy( (ASHashableValue)NULL, m );

	return Success;
}

XVisualInfo *
count_xgetvisualinfo(const char *fname, int line, 
					 Display *dpy, long vinfo_mask, 
					 XVisualInfo *vinfo_template, int *nitems_return)
{
	int my_nitems_return = 0;
	XVisualInfo *val = XGetVisualInfo( dpy, vinfo_mask, vinfo_template, &my_nitems_return );	
	
	if (val != NULL && my_nitems_return)
		count_alloc (fname, line, (void *)val,
					 my_nitems_return * sizeof(XVisualInfo), C_XMEM | C_XGETVISUALINFO);
	*nitems_return = my_nitems_return;		   /* need to do this in case bytes_after_return and nitems_return point to the same var */
	return val;
}

int
count_xgetwindowproperty (const char *fname, int line, Display * display,
						  Window w, Atom property, long long_offset,
						  long long_length, Bool prop_delete, Atom req_type,
						  Atom * actual_type_return,
						  int *actual_format_return,
						  unsigned long *nitems_return, unsigned long *bytes_after_return, unsigned char **prop_return)
{
	int           val;
	unsigned long my_nitems_return;

	val =
		XGetWindowProperty (display, w, property, long_offset, long_length,
							prop_delete, req_type, actual_type_return,
							actual_format_return, &my_nitems_return, bytes_after_return, prop_return);
	if (val == Success && my_nitems_return)
		count_alloc (fname, line, (void *)*prop_return,
					 my_nitems_return * *actual_format_return / 8, C_XMEM | C_XGETWINDOWPROPERTY);
	*nitems_return = my_nitems_return;		   /* need to do this in case bytes_after_return and nitems_return point to the same var */
	return val;
}

Atom *
count_xlistproperties( const char *fname, int line, Display * display,
                       Window w, int *props_num )
{
    Atom *props ;

    props = XListProperties (display, w, props_num);
    if( props != NULL && *props_num > 0 )
        count_alloc (fname, line, (void *)props,
                     (*props_num)*sizeof(Atom), C_XMEM | C_XLISTPROPERTIES);
    return props;
}


Status
count_xgettextproperty(const char *fname, int line, Display * display, Window w,
                       XTextProperty *trg, Atom property)
{
    Status        val;

    val = XGetTextProperty(display,w,trg,property);
    if (val && trg->value )
        count_alloc (fname, line, (void *)(trg->value), strlen((char*)trg->value)+1, C_XMEM | C_XGETTEXTPROPERTY );
    return val;
}

XClassHint *
count_xallocclasshint(const char *fname, int line)
{
    XClassHint *wmclass ;
    wmclass = XAllocClassHint();
    if( wmclass != NULL )
        count_alloc (fname, line, (void *)wmclass, sizeof(XClassHint), C_XMEM | C_XALLOCCLASSHINT );
    return wmclass;
}

XSizeHints *
count_xallocsizehints(const char *fname, int line)
{
    XSizeHints *size_hints ;
    size_hints = XAllocSizeHints();
    if( size_hints != NULL )
        count_alloc (fname, line, (void *)size_hints, sizeof(XSizeHints), C_XMEM | C_XALLOCSIZEHINTS );
    return size_hints;
}


Status
count_xquerytree (const char *fname, int line, Display * display, Window w,
				  Window * root_return, Window * parent_return,
				  Window ** children_return, unsigned int *nchildren_return)
{
	Status        val;

	val = XQueryTree (display, w, root_return, parent_return, children_return, nchildren_return);
	if (val && *nchildren_return)
		count_alloc (fname, line, (void *)*children_return, *nchildren_return * sizeof (Window), C_XMEM | C_XQUERYTREE);
	return val;
}

/* really returns XWMHints*, but to avoid requiring extra includes, we'll return void* */
void         *
count_xgetwmhints (const char *fname, int line, Display * display, Window w)
{
	XWMHints     *val;

	val = XGetWMHints (display, w);
	if (val != NULL)
		count_alloc (fname, line, (void *)val, sizeof (XWMHints), C_XMEM | C_XGETWMHINTS);
	return (void *)val;
}

/* protocols_return is really Atom**, but to avoid extra includes, we'll use void* */
Status
count_xgetwmprotocols (const char *fname, int line, Display * display,
					   Window w, void *protocols_return, int *count_return)
{
	Status        val;

	val = XGetWMProtocols (display, w, (Atom **) protocols_return, count_return);
	if (val && *count_return)
		count_alloc (fname, line, *(void **)protocols_return,
					 *count_return * sizeof (Atom), C_XMEM | C_XGETWMPROTOCOLS);
	return val;
}

/* text_prop_return is really XTextProperty*, but to avoid extra includes, we'll use void* */
Status
count_xgetwmname (const char *fname, int line, Display * display, Window w, void *text_prop_return)
{
	Status        val;
	XTextProperty *prop = text_prop_return;

	val = XGetWMName (display, w, prop);
	if (val && prop->nitems)
		count_alloc (fname, line, (void *)prop->value, prop->nitems * prop->format / 8, C_XMEM | C_XGETWMNAME);
	return val;
}

/* class_hints_return is really XTextProperty*, but to avoid extra includes, we'll use void* */
Status
count_xgetclasshint (const char *fname, int line, Display * display, Window w, void *class_hint_return)
{
	Status        val;
	XClassHint   *prop = class_hint_return;

	val = XGetClassHint (display, w, prop);
	if (val && prop->res_name)
		count_alloc (fname, line, (void *)prop->res_name, strlen (prop->res_name), C_XMEM | C_XGETCLASSHINT);
	if (val && prop->res_class)
		count_alloc (fname, line, (void *)prop->res_class, strlen (prop->res_class), C_XMEM | C_XGETCLASSHINT);
	return val;
}

char         *
count_xgetatomname (const char *fname, int line, Display * display, Atom atom)
{
	char         *val = XGetAtomName (display, atom);

	if (val != NULL)
		count_alloc (fname, line, (void *)val, strlen (val), C_XMEM | C_XGETATOMNAME);
	return val;
}

/* text_prop_return is really XTextProperty*, but to avoid extra includes, we'll use void* */
Status
count_xstringlisttotextproperty (const char *fname, int line, char **list, int count, void *text_prop_return)
{
	Status        val;
	XTextProperty *prop = text_prop_return;

	val = XStringListToTextProperty (list, count, prop);
	if (val && prop->nitems)
		count_alloc (fname, line, (void *)prop->value,
					 prop->nitems * prop->format / 8, C_XMEM | C_XSTRINGLISTTOTEXTPROPERTY);
	return val;
}

int
count_xfree (const char *fname, int line, void *data)
{
	mem          *m = count_find_and_extract (fname, line, (void *)data, C_XMEM);

	if (data == NULL)
	{
		show_error("count_xfree:attempt to free NULL X memory in %s:%d", fname, line);
		return !Success;
	}

	if (m == NULL)
	{
		show_error("count_xfree:attempt in %s:%d to free X memory (%p) that was never allocated or already freed!",
				    fname, line, data);
		return !Success;
	}

	XFree (data);
	mem_destroy( (ASHashableValue)NULL, m );
	return Success;
}

#endif /* #ifndef X_DISPLAY_MISSING */

