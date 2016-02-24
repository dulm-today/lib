#ifndef __STRUCT_DESC_H__
#define __STRUCT_DESC_H__

#include "struct_offset.h"

#define ST_MAX_DEPTH		16


// expand the st_type (struct_offset.h)
enum st_type_ex
{
	st_obj 		= 0x000100,		// st_type_end
	st_array 	= 0x010000,
};

struct st_desc_item;
struct st_desc;

typedef int (*constructor)(void* p);
typedef int (*destructor)(void* p);

typedef struct st_desc_item
{
	const char*		name;		// member name
	
	unsigned		type;		// st_type | st_type_ex
	unsigned 		offset;		// member offset in the struct
	unsigned		array_size;	// the size of array type, if (type & st_array), 0, 1, > 1
	struct st_desc*	child;		// child struct describe, if (type & st_obj), remember the situation (st_ptr | st_obj)
}st_desc_item_t;


typedef struct st_desc
{
	const char*		name;
	int				size;
	int 			item_num;
	st_desc_item_t*	items;

	constructor		fncon;
	destructor		fndes;

	void*			internal;
}st_desc_t;


// pos for find 
typedef struct st_desc_pos
{
	st_desc_t*		desc;
	int				depth;
	st_desc_item_t*	item[ST_MAX_DEPTH];		// item[0] is the pointer of st_desc_t
}st_desc_pos_t;


// structure operate
st_desc_pos_t*	st_find_begin(st_desc_t* desc);
st_desc_pos_t*	st_find_rebegin(st_desc_pos_t* pos);

st_desc_item_t*	st_next(st_desc_pos_t* pos);
st_desc_item_t* st_parent(st_desc_pos_t* pos);
st_desc_item_t* st_child(st_desc_pos_t* pos);

st_desc_item_t* st_get_item(st_desc_pos_t* pos);
st_desc_item_t* st_parent_item(st_desc_pos_t* pos);
st_desc_item_t* st_child_item(st_desc_pos_t* pos);

/*
	name		item name, order by "xxx.xxx"
*/
st_desc_pos_t* 	st_find_item(st_desc_t* desc, const char* name);
void 			st_find_end(st_desc_pos_t* pos);


int	st_is_obj(const st_desc_item_t* item);

int st_is_array(const st_desc_item_t* item);



// object operate
void* st_construct(const st_desc_t* desc);
void  st_destruct(const st_desc_t* desc, void* p);






/*
	p				the pointer of the struct object
*/
char* st_tostring(const st_desc_t* desc, const void* p, char* buf, int len);

/*
	p 				the pointer of the struct member that describe by item
*/
char* st_tostring_item(const st_desc_item_t* item, const void* p, char* buf, int len);


#endif //__STRUCT_DESC_H__
