#include "test_core.h"
#include "heap.h"


struct min_heap_int : public heap_node_t {
	int 	value;

	static int compare(const void* p1, const void* p2) {
		return -(((min_heap_int*)p1)->value - ((min_heap_int*)p2)->value);
	}
};

min_heap_int array[100];


int heap_test()
{
	heap_t hp;

	heap_init(&hp, 8, min_heap_int::compare, NULL);

	for (int i = 0; i < 100; ++i) {
		array[i].value = i;

		heap_push(&hp, &array[i]);
	}

	for (int i = 0; i < 100; ++i) {
		min_heap_int* p = (min_heap_int*)heap_get(&hp, i);
		
		fprintf(stderr, "heap_get %d: value:%d  idx:%d\n", i, p->value,  p->_idx);
	}

	while (!heap_empty(&hp)){
		min_heap_int* p = (min_heap_int*)heap_pop(&hp);

		fprintf(stderr, "heap_pop: value:%d\n", p->value);
	}

	return 0;
}

Test heap_test_item("heap", heap_test, 1);
