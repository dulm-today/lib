#ifndef __STRUCT_H__
#define __STRUCT_H__

#include <stdint.h>

typedef void* (*st_alloc_realloc)(void* blk, void* p, size_t size);
typedef void  (*st_alloc_free)(void* blk, void* p);

typedef struct _st_alloc {
	void*				_blk;
	st_alloc_realloc	_realloc;
	st_alloc_free		_free;
}st_alloc;


typedef int (*compare_fn)(const void* p1, const void* p2);


#endif //__STRUCT_H__