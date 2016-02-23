#include "struct_desc.h"
#include <stdlib.h>


static inline int st_item_in(const st_desc_item_t* item, const st_desc_t* desc)
{
	if (desc->items && item >= desc->items && item <= desc->items + desc->item_num)
		return 1;
	return 0;
}

// structure operate
st_desc_pos_t*	st_find_begin(const st_desc_t* desc)
{
	if (NULL == desc) {
		st_desc_pos_t* pos = (st_desc_pos_t*)malloc(sizeof(st_desc_pos_t));
		if (pos){
			pos->desc = desc;
			pos->depth = 1;
			pos->item[0] = desc->items;
			return pos;
		}
	}
	return NULL;
}

st_desc_pos_t*	st_find_rebegin(st_desc_pos_t* pos)
{
	if (pos) {
		pos->depth = 1;
		pos->item[0] = pos->desc->items;
	}
	return pos;
}


st_desc_item_t*	st_next(st_desc_pos_t* pos)
{
	st_desc_item_t* item = st_get_item(pos);

	if (item) {
		pos->item[pos->depth-1] += 1;
	}

	return item;
}

st_desc_item_t* st_parent(st_desc_pos_t* pos)
{
	if (pos && pos->depth >= 2 && pos->depth <= ST_MAX_DEPTH) {
		return pos->item[--pos->depth - 1];
	}
	return NULL;
}

st_desc_item_t* st_child(st_desc_pos_t* pos)
{
	if (pos && pos->depth > 0 && pos->depth < ST_MAX_DEPTH) {
		if (pos->item[pos->depth-1]->type & st_obj 
			&& pos->item[pos->depth-1]->child){	// ??? st_ptr | st_obj support this?
			st_desc_item_t* item;
			pos->item[pos->depth] = pos->item[pos->depth-1]->child->items;
			pos->depth++;
			item = st_get_item(pos);

			if (!item)
				pos->depth--;
			return 
		}
	}
	return NULL;
}

st_desc_item_t* st_get_item(const st_desc_pos_t* pos)
{	
	if (pos && pos->depth > 0 && pos->depth <= ST_MAX_DEPTH) {
		st_desc_t* desc = NULL;
		
		if (pos->depth == 1){
			desc = pos->desc;
		} else {
			desc= pos->item[pos->depth-2]->child;
		}

		if (st_item_in(pos->item[pos->depth-1], desc))
			return pos->item[pos->depth-1];
	}
	return NULL;
}

st_desc_item_t* st_parent_item(const st_desc_pos_t* pos)
{
	if (pos && pos->depth >= 2 && pos->depth <= ST_MAX_DEPTH) {
		return pos->item[pos->depth - 2];
	}
	return NULL;
}

st_desc_item_t* st_child_item(const st_desc_pos_t* pos);

/*
	name		item name, order by "xxx.xxx"
*/
st_desc_pos_t* 	st_find_item(const st_desc_t* desc, const char* name)
{
	
}


void 			st_find_end(st_desc_pos_t* pos)
{
	if (pos)
		free(pos);
}


int	st_is_obj(const st_desc_item_t* item);

int st_is_array(const st_desc_item_t* item);



// object operate
void* st_construct(const st_desc_t* desc)
{
	void* ptr;
	
	if (NULL == desc)
		return NULL;

	if ((ptr = malloc(desc->size)) == NULL)
		return NULL;

	memset(ptr, 0, desc->size);

	if (!desc->fncon || !desc->fncon(ptr))
		return ptr;

	free(ptr);
	return NULL;
}

void  st_destruct(const st_desc_t* desc, void* p)
{
	if (NULL == desc || NULL == p)
		return;

	if (desc->fndes)
		desc->fndes(p);
	free(p);
}



/*
	p				the pointer of the struct object
*/
char* st_tostring(const st_desc_t* desc, const void* p, char* buf, int len);

/*
	p 				the pointer of the struct member that describe by item
*/
char* st_tostring_item(const st_desc_item_t* item, const void* p, char* buf, int len);

