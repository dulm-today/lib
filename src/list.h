#ifndef __LIST_H__
#define __LIST_H__

#include <assert.h>

typedef struct _list_node{
	struct _list_node* prev;
	struct _list_node* next;
}list_node;

typedef struct _list_head
{
	list_node*	head;
	list_node*  tail;
}list_head;

#define list_entry(listptr, container_type, list_member_name) container_of(listptr, container_type, list_member_name)

#ifdef _MSC_VER
#define container_of(ptr, type, member) ((type*)((char*)(ptr) - offsetof(type, member)))
#else
#define container_of(ptr, type, member) ({ \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
        (type *)( (char *)__mptr - offsetof(type,member) );})
#endif

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#if defined(WIN32) && !defined(__cplusplus)
#define inline __inline
#endif

#ifdef __cplusplus
extern "C"{
#endif

static inline void __list_add( list_node* n, list_node* prev, list_node* next )
{
	prev->next = n;
	next->prev = n;
	n->prev = prev;
	n->next = next;
}

static inline void __list_del(list_node* n)
{
	n->prev->next = n->next;
	n->next->prev = n->prev;
}

static inline int list_empty(const list_head* head)
{
	assert(NULL != head);

	return (NULL == head->head);
}

static inline void list_add_head(list_head* head, list_node* n)
{
	assert(NULL != head && NULL != n);

	if (NULL == head->head){
		n->next = n->prev = n;
		head->tail = n;
	}
	else
		__list_add(n, head->tail, head->head);

	head->head = n;
}

static inline void list_add_tail(list_head* head, list_node* n)
{
	assert(NULL != head && NULL != n);

	if (NULL == head->tail){
		n->next = n->prev = n;
		head->head = n;
	}
	else
		__list_add(n, head->tail, head->head);

	head->tail = n;
}

static inline void list_del(list_head* head, list_node* n)
{
	assert(NULL != head && NULL != n);

	// already removed
	if (NULL == n->next || NULL == n->prev)
		return;

	// list no item
	if (NULL == head->head)
		return;

	// only one item
	if (head->head == head->tail){
		if (head->head == n){
			head->head = head->tail= NULL;
		}
	}
	else{
	
		__list_del(n);

		if (head->head == n)
			head->head = n->next;
		else if (head->tail == n)
			head->tail = n->prev;
	}

	n->next = NULL;
	n->prev = NULL;
}

static inline list_node* list_head_node(const list_head* head)
{
	assert(NULL != head);

	return head->head;
}

static inline list_node* list_tail_node(const list_head* head)
{
	assert(NULL != head);

	return head->tail;
}

static inline list_node* list_for_each(list_head* head, list_node* node)
{
	assert(NULL != head);

	if (NULL == node)
		return head->head;

	if (node != head->tail)
		return node->next;

	return NULL;
}

#ifdef __cplusplus
}
#endif

#endif // __LIST_H__