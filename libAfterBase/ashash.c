/*
 * Copyright (c) 2000 Andrew Ferguson <andrew@owsla.cjb.net>
 * Copyright (c) 1998 Sasha Vasko <sasha@aftercode.net>
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

/*#define DO_CLOCKING      */

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#undef LOCAL_DEBUG

#include "astypes.h"
#include "ashash.h"
#include "safemalloc.h"
#include "output.h"
#include "audit.h"

#define DEALLOC_CACHE_SIZE      1024
static ASHashItem*  deallocated_mem[DEALLOC_CACHE_SIZE+10] ;
static unsigned int deallocated_used = 0 ;

static inline void 
free_ashash_item( ASHashItem *item ) 
{

#ifndef DEBUG_ALLOCS
	if( deallocated_used < DEALLOC_CACHE_SIZE )
    {
    	deallocated_mem[deallocated_used++] = item ;
    }else
#endif
        free( item );
}



ASHashKey default_hash_func (ASHashableValue value, ASHashKey hash_size)
{
	return value % hash_size;
}

long
default_compare_func (ASHashableValue value1, ASHashableValue value2)
{
	return ((long)value1 - (long)value2);
}

long
desc_long_compare_func (ASHashableValue value1, ASHashableValue value2)
{
    return ((long)value2 - (long)value1);
}

void
init_ashash (ASHashTable * hash, Bool freeresources)
{
	LOCAL_DEBUG_CALLER_OUT( " hash = %p, free ? %d", hash, freeresources );
	if (hash)
	{
		if (freeresources)
			if (hash->buckets)
				free (hash->buckets);
		memset (hash, 0x00, sizeof (ASHashTable));
	}
}

ASHashTable  *
create_ashash (ASHashKey size,
			   ASHashKey (*hash_func) (ASHashableValue, ASHashKey),
			   long (*compare_func) (ASHashableValue, ASHashableValue),
			   void (*item_destroy_func) (ASHashableValue, void *))
{
	ASHashTable  *hash;

	LOCAL_DEBUG_CALLER_OUT( " size = %d", size );

	if (size <= 0)
		size = DEFAULT_HASH_SIZE;

	hash = safemalloc (sizeof (ASHashTable));

	init_ashash (hash, False);

	hash->buckets = safecalloc (size, sizeof (ASHashBucket));
	hash->size = size;

	if (hash_func)
		hash->hash_func = hash_func;
	else
		hash->hash_func = default_hash_func;

	if (compare_func)
		hash->compare_func = compare_func;
	else
		hash->compare_func = default_compare_func;

	hash->item_destroy_func = item_destroy_func;

	return hash;
}

static void
destroy_ashash_bucket (ASHashBucket * bucket, void (*item_destroy_func) (ASHashableValue, void *))
{
	register ASHashItem *item, *next;

	for (item = *bucket; item != NULL; item = next)
	{
		next = item->next;
		if (item_destroy_func)
			item_destroy_func (item->value, item->data);
		free_ashash_item(item);
	}
	*bucket = NULL;
}

void
flush_ashash (ASHashTable * hash)
{
LOCAL_DEBUG_CALLER_OUT( " hash = %p", hash  );
	if (hash)
	{
		register int  i = hash->size;
		while( --i >= 0 )
			if (hash->buckets[i])
				destroy_ashash_bucket (&(hash->buckets[i]), hash->item_destroy_func);
		hash->items_num = 0 ;
		hash->buckets_used = 0 ;
		hash->most_recent = NULL ;
	}
}

void
destroy_ashash (ASHashTable ** hash)
{
LOCAL_DEBUG_CALLER_OUT( " hash = %p, *hash = %p", hash, *hash  );
	if (*hash)
	{
		flush_ashash (*hash);
		init_ashash (*hash, True);
		free (*hash);
		*hash = NULL;
	}
}

static        ASHashResult
add_item_to_bucket (ASHashBucket * bucket, ASHashItem * item, long (*compare_func) (ASHashableValue, ASHashableValue))
{
	ASHashItem  **tmp;

	/* first check if we already have this item */
	for (tmp = bucket; *tmp != NULL; tmp = &((*tmp)->next))
	{
		register long res = compare_func ((*tmp)->value, item->value);

		if (res == 0)
			return ((*tmp)->data == item->data) ? ASH_ItemExistsSame : ASH_ItemExistsDiffer;
		else if (res > 0)
			break;
	}
	/* now actually add this item */
	item->next = (*tmp);
	*tmp = item;
	return ASH_Success;
}

#ifdef DEBUG_ALLOCS
#undef add_hash_item
#undef safecalloc
void* safecalloc(size_t,size_t);
#undef free
#endif

Bool
check_hash_item_reused (ASHashItem *item)
{
/*	fprintf( stderr, "item = %p, used = %d, mem = %p\n", item, deallocated_used, deallocated_mem[deallocated_used] ); */
	if( deallocated_used >= DEALLOC_CACHE_SIZE )
		return False;
	return (deallocated_mem[deallocated_used] == item );
}

ASHashResult
add_hash_item (ASHashTable * hash, ASHashableValue value, void *data)
{
	ASHashKey     key;
	ASHashItem   *item;
	ASHashResult  res;

	if (hash == NULL)
        return ASH_BadParameter;
	key = hash->hash_func (value, hash->size);
	if (key >= hash->size)
        return ASH_BadParameter;

#ifndef DEBUG_ALLOCS
    if( deallocated_used > 0 )
        item = deallocated_mem[--deallocated_used];
    else
#endif	
        item = safecalloc (1, sizeof (ASHashItem));

	LOCAL_DEBUG_OUT( "hash %p, value 0x%lX, data = %p, item = %p ", hash, value, data, item );

	item->next = NULL;
	item->value = value;
	item->data = data;

	res = add_item_to_bucket (&(hash->buckets[key]), item, hash->compare_func);
	if (res == ASH_Success)
	{
		hash->most_recent = item ;
		hash->items_num++;
		if (hash->buckets[key]->next == NULL)
			hash->buckets_used++;
	} else
		free_ashash_item(item);
	return res;
}

#ifdef DEBUG_ALLOCS
#define safecalloc(a,b) countcalloc(__FUNCTION__, __LINE__, a,b)
#define add_hash_item(a,b,c) countadd_hash_item(__FUNCTION__, __LINE__,a,b,c)
#define free(a) countfree(__FUNCTION__, __LINE__, a)
#endif
void
print_ashash (ASHashTable * hash, void (*item_print_func) (ASHashableValue value))
{
	register int  i;
	ASHashItem   *item;

	for (i = 0; i < hash->size; i++)
	{
		if (hash->buckets[i] == NULL)
			continue;
		fprintf (stderr, "Bucket # %d:", i);
		for (item = hash->buckets[i]; item != NULL; item = item->next)
			if (item_print_func)
				item_print_func (item->value);
			else
				fprintf (stderr, "[0x%lX(%ld)]", item->value, item->value);
		fprintf (stderr, "\n");
	}
}

void
print_ashash2 (ASHashTable * hash, void (*item_print_func) (ASHashableValue value, void *data))
{
	register int  i;
	ASHashItem   *item;

	for (i = 0; i < hash->size; i++)
	{
		if (hash->buckets[i] == NULL)
			continue;
		fprintf (stderr, "Bucket # %d:", i);
		for (item = hash->buckets[i]; item != NULL; item = item->next)
			if (item_print_func)
				item_print_func (item->value, item->data);
			else
				fprintf (stderr, "[0x%lX(%ld):%p]", item->value, item->value, item->data);
		fprintf (stderr, "\n");
	}
}


static ASHashItem **
find_item_in_bucket (ASHashBucket * bucket,
					 ASHashableValue value, long (*compare_func) (ASHashableValue, ASHashableValue))
{
	register ASHashItem **tmp;
	register long res;

	/* first check if we already have this item */
	for (tmp = bucket; *tmp != NULL; tmp = &((*tmp)->next))
	{
		res = compare_func ((*tmp)->value, value);
		if (res == 0)
			return tmp;
		else if (res > 0)
			break;
	}
	return NULL;
}

ASHashResult
get_hash_item (ASHashTable * hash, ASHashableValue value, void **trg)
{
	ASHashKey     key;
	ASHashItem  **pitem = NULL;

	if (hash)
	{
		key = hash->hash_func (value, hash->size);
		if (key < hash->size)
			pitem = find_item_in_bucket (&(hash->buckets[key]), value, hash->compare_func);
	}
	if (pitem)
		if (*pitem)
		{
			if (trg)
				*trg = (*pitem)->data;
			return ASH_Success;
		}
	return ASH_ItemNotExists;
}

void flush_ashash_memory_pool()
{
	/* we better disable errors as some of this data will belong to memory audit : */
    int old_cleanup_mode = set_audit_cleanup_mode(1);
	while( deallocated_used > 0 )
		free( deallocated_mem[--deallocated_used] );
	set_audit_cleanup_mode(old_cleanup_mode);
}

ASHashResult
remove_hash_item (ASHashTable * hash, ASHashableValue value, void **trg, Bool destroy)
{
	ASHashKey     key = 0;
	ASHashItem  **pitem = NULL;

	if (hash)
	{
		key = hash->hash_func (value, hash->size);
		if (key < hash->size)
			pitem = find_item_in_bucket (&(hash->buckets[key]), value, hash->compare_func);
	}
	if (pitem)
		if (*pitem)
		{
			ASHashItem   *next;

			if( hash->most_recent == *pitem )
				hash->most_recent = NULL ;

			if (trg)
				*trg = (*pitem)->data;

			next = (*pitem)->next;
			if (hash->item_destroy_func && destroy)
				hash->item_destroy_func ((*pitem)->value, (trg) ? NULL : (*pitem)->data);

			free_ashash_item(*pitem);

            *pitem = next;
			if (hash->buckets[key] == NULL)
				hash->buckets_used--;
			hash->items_num--;

			return ASH_Success;
		}
	return ASH_ItemNotExists;
}

unsigned long
sort_hash_items (ASHashTable * hash, ASHashableValue * values, void **data, unsigned long max_items)
{
	if (hash)
	{
		ASHashBucket *buckets;
		register ASHashKey i;
		ASHashKey     k = 0, top = hash->buckets_used - 1, bottom = 0;
		unsigned long count_in = 0;

		if (hash->buckets_used == 0 || hash->items_num == 0)
			return 0;

		if (max_items == 0)
			max_items = hash->items_num;
		
		buckets = safecalloc (hash->buckets_used, sizeof (ASHashBucket));
		for (i = 0; i < hash->size; i++)
			if (hash->buckets[i])
				buckets[k++] = hash->buckets[i];
		while (max_items-- > 0)
		{
			k = bottom;
			for (i = bottom + 1; i <= top; i++)
			{
				if (buckets[i])
					if (hash->compare_func (buckets[k]->value, buckets[i]->value) > 0)
						k = i;
			}
            if (values)
				*(values++) = buckets[k]->value;
			if (data)
				*(data++) = buckets[k]->data;
			count_in++;
			buckets[k] = buckets[k]->next;
			/* a little optimization : */
			while (buckets[bottom] == NULL && bottom < top)
				bottom++;
			while (buckets[top] == NULL && top > bottom)
				top--;
			if( buckets[top] == NULL )
				break;
		}
		free (buckets);
		return count_in;
	}
	return 0;
}

unsigned long
list_hash_items (ASHashTable * hash, ASHashableValue * values, void **data, unsigned long max_items)
{
	unsigned long count_in = 0;

	if (hash)
		if (hash->buckets_used > 0 && hash->items_num > 0)
		{
			register ASHashItem *item;
			ASHashKey     i;

			if (max_items == 0)
				max_items = hash->items_num;

			for (i = 0; i < hash->size; i++)
				for (item = hash->buckets[i]; item != NULL; item = item->next)
				{
					if (values)
						*(values++) = item->value;
					if (data)
						*(data++) = item->data;
					if (++count_in >= max_items)
						return count_in;
				}
		}
	return count_in;
}

/***** Iterator functionality *****/

Bool
start_hash_iteration (ASHashTable * hash, ASHashIterator * iterator)
{
	if (iterator && hash)
	{
		register int  i;

		for (i = 0; i < hash->size; i++)
			if (hash->buckets[i] != NULL)
				break;
		if (i < hash->size)
		{
			iterator->hash = hash;
			iterator->curr_bucket = i;
            iterator->curr_item = &(hash->buckets[i]);
			return True;
		}
	}
	return False;
}

Bool
next_hash_item (ASHashIterator * iterator)
{
	if (iterator)
		if (iterator->hash && iterator->curr_item)
		{
            ASHashItem **curr = iterator->curr_item;

            if( *curr )
                curr = &((*curr)->next) ;

            iterator->curr_item = curr ;

            if (*curr == NULL)
			{
				register int  i;

				for (i = iterator->curr_bucket + 1; i < iterator->hash->size; i++)
					if (iterator->hash->buckets[i] != NULL)
						break;
				if (i < iterator->hash->size)
				{
                    iterator->curr_item = &(iterator->hash->buckets[i]);
					iterator->curr_bucket = i ;
                }
            }

            return (*(iterator->curr_item) != NULL);
		}
	return False;
}

void
remove_curr_hash_item (ASHashIterator * iterator, Bool destroy)
{
	if (iterator)
		if (iterator->hash && iterator->curr_item)
		{
            ASHashItem *removed = *(iterator->curr_item);

            if(removed)
            {
                ASHashTable *hash = iterator->hash ;
                ASHashKey    key = iterator->curr_bucket ;

                *(iterator->curr_item) = removed->next ;
                removed->next = NULL ;

                if (hash->item_destroy_func && destroy)
                    hash->item_destroy_func (removed->value, removed->data);
				free_ashash_item(removed);
                if (hash->buckets[key] == NULL)
                    hash->buckets_used--;
                hash->items_num--;
            }
		}
}

inline ASHashableValue
curr_hash_value (ASHashIterator * iterator)
{
	if (iterator)
	{
        if (iterator->curr_item && *(iterator->curr_item))
            return (*(iterator->curr_item))->value;
	}
	return (ASHashableValue) ((char *)NULL);
}

inline void         *
curr_hash_data (ASHashIterator * iterator)
{
	if (iterator)
	{
        if (iterator->curr_item && *(iterator->curr_item))
            return (*(iterator->curr_item))->data;
	}
	return NULL;
}

/************************************************************************/
/************************************************************************/
/* 	Some usefull implementations 					*/
/************************************************************************/
ASHashKey pointer_hash_value (ASHashableValue value, ASHashKey hash_size)
{
    union
    {
        void *ptr;
        ASHashKey key[2];
    } mix;
    register  ASHashKey key;

    mix.ptr = (void*)value;
    key = mix.key[0]^mix.key[1] ;
    if( hash_size == 256 )
		return (key>>4)&0x0FF;
    return (key>>4) % hash_size;
}

/* case sensitive strings hash */
ASHashKey
string_hash_value (ASHashableValue value, ASHashKey hash_size)
{
	ASHashKey     hash_key = 0;
	register int  i = 0;
	char         *string = (char*)value;
	register char c;

	do
	{
		if( (c=string[i]) != 0)
		{
			hash_key += (ASHashKey) c;
			if( string[++i] != 0 && (c=string[++i]) != 0)
			{
				hash_key += ((ASHashKey) c)<<1;
				if( string[++i] != 0 && (c=string[++i]) != 0)
				{
					hash_key += ((ASHashKey) c)<<2;
					if( string[++i] != 0 && (c=string[++i]) != 0)
					{
						hash_key += ((ASHashKey) c)<<3;
					}
				}
			}
		}
	}while(c != 0 && i < 32 );
	return hash_key % hash_size;
}

long
string_compare (ASHashableValue value1, ASHashableValue value2)
{
	register char *str1 = (char*)value1;
	register char *str2 = (char*)value2;
	register int   i = 0 ;

	if (str1 == str2)
		return 0;
	if (str1 == NULL)
		return -1;
	if (str2 == NULL)
		return 1;
	do
	{
		if (str1[i] != str2[i])
			return (long)(str1[i]) - (long)(str2[i]);

	}while( str1[i++] );
	return 0;
}

void
string_destroy (ASHashableValue value, void *data)
{
	if ((char*)value != NULL)
		free ((char*)value);
	if (data != (void*)value && data != NULL)
		free (data);
}

void
string_destroy_without_data (ASHashableValue value, void *data)
{
	if ((char*)value != NULL)
		free ((char*)value);
}

void
string_print (ASHashableValue value)
{
	fprintf (stderr, "[%s]", (char*)value);
}

/* variation for case-unsensitive strings */
ASHashKey
casestring_hash_value (ASHashableValue value, ASHashKey hash_size)
{
	ASHashKey     hash_key = 0;
	register int  i = 0;
	char         *string = (char*)value;
	register int c;

	do
	{
		if( (c=string[i]) != 0)
		{
			if (isupper (c))c = tolower (c);
			hash_key += (ASHashKey) c;
			if( string[++i] != 0 && (c=string[++i]) != 0)
			{
				if (isupper (c))c = tolower (c);
				hash_key += ((ASHashKey) c)<<1;
				if( string[++i] != 0 && (c=string[++i]) != 0)
				{
					if (isupper (c))c = tolower (c);
					hash_key += ((ASHashKey) c)<<2;
					if( string[++i] != 0 && (c=string[++i]) != 0)
					{
						if (isupper (c))c = tolower (c);
						hash_key += ((ASHashKey) c)<<3;
					}
				}
			}
		}
	}while(c != 0 && i < 32 );

	return hash_key % hash_size;
}

long
casestring_compare (ASHashableValue value1, ASHashableValue value2)
{
	register char *str1 = (char*)value1;
	register char *str2 = (char*)value2;
	register int   i = 0;

	if (str1 == str2)
		return 0;
	if (str1 == NULL)
		return -1;
	if (str2 == NULL)
		return 1;
	do
	{
		int          u1, u2;

		u1 = str1[i];
		u2 = str2[i];
		if (islower (u1))
			u1 = toupper (u1);
		if (islower (u2))
			u2 = toupper (u2);
		if (u1 != u2)
			return (long)u1 - (long)u2;
	}while( str1[i++] );
	return 0;
}

/* variation for config option type of strings */
ASHashKey
option_hash_value (ASHashableValue value, ASHashKey hash_size)
{
	ASHashKey     hash_key = 0;
	register int  i = 0;
	char         *opt = (char*)value;
	register char c;

	do
	{
		c = opt[i];
#define VALID_OPTION_CHAR(c)		(isalnum ((int)c) || (c) == '~' || (c) == '_')
		if (c == '\0' || !VALID_OPTION_CHAR(c))
			break;
		if (isupper ((int)c))
			c = tolower ((int)c);
		hash_key += (((ASHashKey) c) << i);
		++i;
	}while( i < ((sizeof (ASHashKey) - sizeof (char)) << 3) );
	return hash_key % hash_size;
}

long
option_compare (ASHashableValue value1, ASHashableValue value2)
{
	char *str1 = (char*)value1;
	char *str2 = (char*)value2;
	register int i = 0;

	if (str1 == str2)
		return 0;
	if (str1 == NULL)
		return -1;
	if (str2 == NULL)
		return 1;
	while ( str1[i] && str2[i] )
	{
		register int          u1, u2;

		u1 = str1[i];
		u2 = str2[i];
		if( !VALID_OPTION_CHAR(u1) )
			return ( VALID_OPTION_CHAR(u2) )? (long)u1 - (long)u2: 0;
		++i ;

		if (islower (u1))
			u1 = toupper (u1);
		if (islower (u2))
			u2 = toupper (u2);
		if (u1 != u2)
			return (long)u1 - (long)u2;
	}
	if(  str1[i] == str2[i] )
		return 0;
	else if( str1[i] )
		return ( VALID_OPTION_CHAR(str1[i]) )?(long)(str1[i]):0;
	else
		return ( VALID_OPTION_CHAR(str2[i]) )? 0 - (long)(str2[i]):0;
}


/* colors hash */

ASHashKey
color_hash_value (ASHashableValue value, ASHashKey hash_size)
{
	register CARD32 h;
	int           i = 1;
	register unsigned long long_val = value;

	h = (long_val & 0x0ff);
	long_val = long_val >> 8;

	while (i++ < 4)
	{										   /* for the first 4 bytes we can use simplier loop as there can be no overflow */
		h = (h << 4) + (long_val & 0x0ff);
		long_val = long_val >> 8;
	}
	while (i++ < sizeof (unsigned long))
	{										   /* this loop will only be used on computers with long > 32bit */
		register CARD32 g;

		h = (h << 4) + (long_val & 0x0ff);
		long_val = long_val >> 8;
		if ((g = h & 0xf0000000))
		{
			h ^= g >> 24;
			h &= 0x0fffffff;
		}
	}
	return (ASHashKey) (h % (CARD32) hash_size);
}
