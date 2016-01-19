#include "test_core.h"
#include "list.h"
#include <stdio.h>

struct item
{
	list_node node;
	int		value;
};

void list_print(const char* title, list_head* head)
{
	list_node* node = NULL;
	
	fprintf(stderr, "%s: list: ", title);
	while(NULL != (node = list_for_each(head, node)))
	{
		struct item* item_node = container_of(node, struct item, node);
		
		fprintf(stderr, "%d ", item_node->value);
	}
	
	fprintf(stderr, "\n");
}

int list_check(const char* title, list_head* head, const int* va, int size)
{
	int index = 0;
	list_node* node = NULL;
	
	fprintf(stderr, "%s: list_check: ", title);
	
	while(NULL != (node = list_for_each(head, node)))
	{
		struct item* item_node = container_of(node, struct item, node);
		
		fprintf(stderr, "[%d ", item_node->value);
		
		if (index >= size)
			goto err;
			
		if (item_node->value != va[index])
			goto err;
			
		fprintf(stderr, "%d] ", va[index]);
		index++;
	}
	
	if (index != size){
		fprintf(stderr, "index[%d] != size[%d]\n", index, size);
		return -1;
	}
	
	fprintf(stderr, "\n");
	return 0;
	
err:
	fprintf(stderr, "\n");
	return -1;
}

int list_add_head_test()
{
	int index = 0;
	list_head head = {0};
	int va[] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
	
	for (;index < 10; ++index)
	{
		struct item* item_node = new struct item;
		item_node->value = index;
		
		list_add_head(&head, &item_node->node);
	}
	
	list_print("list_add_head", &head);
	
	return list_check("list_add_head", &head, va, sizeof(va)/sizeof(va[0]));
}

int list_add_tail_test()
{
	int index = 0;
	list_head head = {0};
	int va[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	
	for (;index < 10; ++index)
	{
		struct item* item_node = new struct item;
		item_node->value = index;
		
		list_add_tail(&head, &item_node->node);
	}
	
	list_print("list_add_tail", &head);
	
	return list_check("list_add_tail", &head, va, sizeof(va)/sizeof(va[0]));
}


int list_del_test()
{
	int index = 0;
	list_head head = {0};
	int va[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	
	for (;index < 10; ++index)
	{
		struct item* item_node = new struct item;
		item_node->value = index;
		
		list_add_tail(&head, &item_node->node);
	}
	
	list_print("list_del", &head);
	
	for (index = 9;index >= 0; --index)
	{
		struct item* item_node = container_of(list_tail_node(&head), struct item, node);
		
		list_del(&head, &item_node->node);
		
		list_print("list_del", &head);
		
		if (index == 0)
		{
			if (!list_empty(&head)){
				fprintf(stderr, "list is not empty!\n");
				return -1;
			}
		}
		else if (list_check("list_del", &head, va, index))
			return -1;
	}
	
	return 0;
}


int list_test()
{
	int error = 0;
	
	if (list_add_head_test())
		error++;
		
	if (list_add_tail_test())
		error++;
		
	if (list_del_test())
		error++;
	
	return error;
}

Test list_item("list", list_test, 3);