#ifndef __HEAP_H__
#define __HEAP_H__

#include "struct.h"


#ifdef __cplusplus
extern "C" {
#endif

typedef struct _heap_node{
	unsigned 	_idx;
}heap_node_t;

typedef struct _heap{
	heap_node_t** _ptr;
	unsigned	_alloc;
	unsigned	_num;

	compare_fn	_cmp;
	st_alloc 	_st_alloc;
}heap_t;


void heap_init(heap_t* hp, unsigned size, compare_fn cmp, const st_alloc* alloc);
void heap_release(heap_t* hp);

void* heap_top(heap_t* hp);
void* heap_pop(heap_t* hp);
int   heap_push(heap_t* hp, void* p);
void* heap_get(heap_t* hp, unsigned idx);
int   heap_erase(heap_t* hp, unsigned idx);

int   heap_reserve(heap_t* hp, unsigned size);
unsigned heap_size(heap_t* hp);
int   heap_empty(heap_t* hp);



#ifdef __cplusplus
}
#endif
#endif // __HEAP_H__