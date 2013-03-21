#ifndef ASVECTOR_HEADER_FILE_INCLUDED
#define ASVECTOR_HEADER_FILE_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ASVector
{
    void *memory ;
    size_t allocated, used ;
    size_t unit ;
}ASVector;

#define DECL_VECTOR(t,v)    ASVector v = {NULL,0,0,sizeof(t)}
#define DECL_VECTOR_RAW(v)  ASVector v = {NULL,0,0,1}
#define VECTOR_HEAD(t,v)    ((t*)((v).memory))
#define PVECTOR_HEAD(t,v)    ((t*)((v)->memory))
#define VECTOR_HEAD_RAW(v)  ((v).memory)
#define VECTOR_TAIL(t,v)    (((t*)((v).memory)) + ((v).used))
#define PVECTOR_TAIL(t,v)    (((t*)((v)->memory)) + ((v)->used))
#define VECTOR_TAIL_RAW(v)  (((v).memory) + ((v).used*(v).unit))
#define VECTOR_USED(v)      ((v).used)
#define VECTOR_UNIT(v)      ((v).unit)
#define PVECTOR_USED(v)      ((v)->used)
#define PVECTOR_UNIT(v)      ((v)->unit)

ASVector *create_asvector( size_t unit );
void destroy_asvector( ASVector **v );
/* return memory starting address : */
void *alloc_vector( ASVector *v, size_t new_size );
void *realloc_vector( ASVector *v, size_t new_size );
/* If append_vector is used with data == NULL it will only allocate
   additional space, but will not copy any data.
   Returns self : */
ASVector *append_vector( ASVector *v, void * data, size_t size );

/* returns index on success, -1 on failure */
int vector_insert_elem( ASVector *v, void *data, size_t size, void *sibling, int before );
int vector_relocate_elem (ASVector *v, unsigned int index, unsigned int new_index);
inline size_t vector_find_data (ASVector *v, void *data );
int vector_find_elem( ASVector *v, void *data );
/* returns 1 on success, 0 on failure */
int vector_remove_elem( ASVector *v, void *data );
int vector_remove_index( ASVector *v, size_t index );

void
print_vector( stream_func func, void* stream, ASVector *v, char *name, void (*print_func)( stream_func func, void* stream, void *data ) );


/* returns nothing : */
void flush_vector( ASVector *v ); /* reset used to 0 */
void free_vector( ASVector *v );  /* free all memory used, except for object itself */

#ifdef __cplusplus
}
#endif


#endif /* #ifndef ASVECTOR_HEADER_FILE_INCLUDED */
