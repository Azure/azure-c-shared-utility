#ifndef MEMORY_H_
#define MEMORY_H_

#include <stddef.h>

#define malloc  malloc_wrapper
#define calloc  calloc_wrapper
#define realloc realloc_wrapper
#define free    free_wrapper

void* malloc_wrapper ( size_t size );
void* calloc_wrapper ( size_t nmemb , size_t size );
void* realloc_wrapper ( void* ptr , size_t size );
void free_wrapper ( void* ptr );

#endif /* MEMORY_H_ */
