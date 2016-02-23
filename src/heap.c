#include "heap.h"
#include <stdlib.h>
#include <string.h>

#define HEAP_ELE(hp, idx)	(hp)->_ptr[idx]

static void* heap_realloc(void* blk, void* p, size_t size)
{	
	return realloc(p, size);
}

static void heap_free(void* blk, void* p)
{
	free(p); 
}

static void heap_shift_down(heap_t* hp, unsigned idx, heap_node_t* p)
{
	unsigned min_child = (idx + 1) * 2;
	while (min_child <= hp->_num){
		min_child -= (hp->_num == min_child || hp->_cmp(HEAP_ELE(hp, min_child), HEAP_ELE(hp, min_child-1)) > 0);

		if (hp->_cmp(HEAP_ELE(hp, min_child), p) >= 0)
			break;

		(HEAP_ELE(hp, idx) = HEAP_ELE(hp, min_child))->_idx = idx;
		idx = min_child;
		min_child = (idx + 1) * 2;
	}

	(HEAP_ELE(hp, idx) = p)->_idx = idx;
}

static void heap_shift_up(heap_t* hp, unsigned idx, heap_node_t* p)
{
	unsigned parent = (idx - 1) / 2;
	while (idx && hp->_cmp(HEAP_ELE(hp, parent), p) > 0){
		(HEAP_ELE(hp, idx) = HEAP_ELE(hp, parent))->_idx = idx;
		idx = parent;
		parent = (idx - 1) / 2;
	}
	(HEAP_ELE(hp, idx) = p)->_idx = idx;
}

void heap_init(heap_t* hp, unsigned size, compare_fn cmp, const st_alloc* alloc)
{
	memset(hp, 0, sizeof(heap_t));

	hp->_cmp = cmp;

	if (alloc)
		memcpy(&hp->_st_alloc, alloc, sizeof(st_alloc));

	if (!hp->_st_alloc._realloc || !hp->_st_alloc._free) {
		hp->_st_alloc._realloc = heap_realloc;
		hp->_st_alloc._free = heap_free;
	}

	heap_reserve(hp, size);
}

void heap_release(heap_t* hp)
{
	if (hp->_ptr) {
		hp->_st_alloc._free(hp->_st_alloc._blk, hp->_ptr);
	}

	memset(hp, 0, sizeof(heap_t));
}

void* heap_top(heap_t* hp)
{
	return hp->_num ? HEAP_ELE(hp, 0) : 0;
}

void* heap_pop(heap_t* hp)
{
	if (hp->_num) {
		void* p = HEAP_ELE(hp, 0);
		heap_shift_down(hp, 0, HEAP_ELE(hp, --hp->_num));
		((heap_node_t*)p)->_idx = -1;
		return p;
	}

	return 0;
}

int   heap_push(heap_t* hp, void* p)
{
	if (heap_reserve(hp, hp->_num + 1))
		return -1;

	heap_shift_up(hp, hp->_num++, p);

	return 0;
}

void* heap_get(heap_t* hp, unsigned idx)
{
	if (idx < hp->_num) {
		return HEAP_ELE(hp, idx);
	}

	return 0;
}

int   heap_erase(heap_t* hp, unsigned idx)
{
	if (idx < hp->_num) {
		heap_shift_down(hp, idx, HEAP_ELE(hp, --hp->_num));
	}

	return -1;
}

int   heap_reserve(heap_t* hp, unsigned size)
{
	if (hp->_alloc < size){
		void* p;
		unsigned alloc = hp->_alloc ? hp->_alloc * 2 : size;

		if (alloc < size)
			alloc = size;

		p = hp->_st_alloc._realloc(hp->_st_alloc._blk, hp->_ptr, alloc * sizeof(void*));
		if (!p)
			return -1;

		hp->_ptr = p;
		hp->_alloc = alloc;
	}
	
	return 0;
}


unsigned heap_size(heap_t* hp)
{
	return hp->_num;
}

int   heap_empty(heap_t* hp)
{
	return hp->_num == 0;
}

