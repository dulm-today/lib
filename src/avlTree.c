/*
    avl_tree.c -- avl_ tree and linked list convenience
	Copyright (C) 1998 Michael H. Buselli
				  2000-2003 Ivo Timmermans <ivo@o2w.nl>,
				  2000-2003 Guus Sliepen <guus@sliepen.eu.org>
				  2000-2003 Wessel Dankers <wsl@nl.linux.org>

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

	Original AVL tree library by Michael H. Buselli <cosine@cosine.org>.

	Modified 2000-11-28 by Wessel Dankers <wsl@nl.linux.org> to use counts
	instead of depths, to add the ->next and ->prev and to generally obfuscate
	the code. Mail me if you found a bug.

	Cleaned up and incorporated some of the ideas from the red-black tree
	library for inclusion into tinc (http://tinc.nl.linux.org/) by
	Guus Sliepen <guus@sliepen.eu.org>.

	$Id: avl_tree.c,v 1.1.2.19 2003/08/28 21:05:09 guus Exp $
*/

#include "avlTree.h"

#ifndef NULL
#define NULL 0
#endif

#ifdef AVL_COUNT
#define AVL_NODE_COUNT(n)  ((n) ? (n)->count : 0)
#define AVL_L_COUNT(n)	 (AVL_NODE_COUNT((n)->left))
#define AVL_R_COUNT(n)	 (AVL_NODE_COUNT((n)->right))
#define AVL_CALC_COUNT(n)  (AVL_L_COUNT(n) + AVL_R_COUNT(n) + 1)
#endif

#ifdef AVL_DEPTH
#define AVL_NODE_DEPTH(n)  ((n) ? (n)->depth : 0)
#define L_AVL_DEPTH(n)	 (AVL_NODE_DEPTH((n)->left))
#define R_AVL_DEPTH(n)	 (AVL_NODE_DEPTH((n)->right))
#define AVL_CALC_DEPTH(n)  ((L_AVL_DEPTH(n)>R_AVL_DEPTH(n)?L_AVL_DEPTH(n):R_AVL_DEPTH(n)) + 1)
#endif

#ifndef AVL_DEPTH
static int lg(unsigned int u) __attribute__ ((__const__));

static int lg(unsigned int u)
{
	int r = 1;

	if(!u)
		return 0;

	if(u & 0xffff0000) {
		u >>= 16;
		r += 16;
	}

	if(u & 0x0000ff00) {
		u >>= 8;
		r += 8;
	}

	if(u & 0x000000f0) {
		u >>= 4;
		r += 4;
	}

	if(u & 0x0000000c) {
		u >>= 2;
		r += 2;
	}

	if(u & 0x00000002)
		r++;

	return r;
}
#endif

/* Internal helper functions */

static int avl_check_balance(const avl_node_t *node)
{
#ifdef AVL_DEPTH
	int d;

	d = R_AVL_DEPTH(node) - L_AVL_DEPTH(node);

	return d < -1 ? -1 : d > 1 ? 1 : 0;
#else
/*      int d;
 *      d = lg(AVL_R_COUNT(node)) - lg(AVL_L_COUNT(node));
 *      d = d<-1?-1:d>1?1:0;
 */
	int pl, r;

	pl = lg(AVL_L_COUNT(node));
	r = AVL_R_COUNT(node);

	if(r >> pl + 1)
		return 1;

	if(pl < 2 || r >> pl - 2)
		return 0;

	return -1;
#endif
}

static void avl_rebalance(avl_tree_t *tree, avl_node_t *node)
{
	avl_node_t *child;
	avl_node_t *gchild;
	avl_node_t *parent;
	avl_node_t **superparent;

	//parent = node;	edit by dulm

	while(node) {
		parent = node->parent;

		superparent =
			parent ? node ==
			parent->left ? &parent->left : &parent->right : &tree->root;

		switch (avl_check_balance(node)) {
			case -1:
				child = node->left;
#ifdef AVL_DEPTH
				if(L_AVL_DEPTH(child) >= R_AVL_DEPTH(child))
#else
				if(AVL_L_COUNT(child) >= AVL_R_COUNT(child))
#endif
				{// LL型平衡旋转, node的左子节点 向右上做一次顺时针旋转
					node->left = child->right;
					if(node->left)
						node->left->parent = node;

					child->right = node;
					node->parent = child;
					*superparent = child;
					child->parent = parent;
#ifdef AVL_COUNT
					node->count = AVL_CALC_COUNT(node);
					child->count = AVL_CALC_COUNT(child);
#endif
#ifdef AVL_DEPTH
					node->depth = AVL_CALC_DEPTH(node);
					child->depth = AVL_CALC_DEPTH(child);
#endif
				} else {
					// LR型平衡旋转, 1. node的左子节点的右子节点 向左上做一次逆时针旋转,得到LL型
					//               2. node的左子节点向右上做一次顺时针旋转。
					// 这里的实现是直接根据2次旋转的结果进行赋值
					gchild = child->right;
					
					node->left = gchild->right;
					if(node->left)
						node->left->parent = node;
					
					child->right = gchild->left;
					if(child->right)
						child->right->parent = child;
					
					gchild->right = node;
					if(gchild->right)
						gchild->right->parent = gchild;
					
					gchild->left = child;
					if(gchild->left)
						gchild->left->parent = gchild;
					
					*superparent = gchild;

					gchild->parent = parent;
#ifdef AVL_COUNT
					node->count = AVL_CALC_COUNT(node);
					child->count = AVL_CALC_COUNT(child);
					gchild->count = AVL_CALC_COUNT(gchild);
#endif
#ifdef AVL_DEPTH
					node->depth = AVL_CALC_DEPTH(node);
					child->depth = AVL_CALC_DEPTH(child);
					gchild->depth = AVL_CALC_DEPTH(gchild);
#endif
				}
				break;

			case 1:
				child = node->right;
#ifdef AVL_DEPTH
				if(R_AVL_DEPTH(child) >= L_AVL_DEPTH(child))
#else
				if(AVL_R_COUNT(child) >= AVL_L_COUNT(child))
#endif
				{// RR型平衡旋转, node的右子节点向左上做一次逆时针旋转
					node->right = child->left;
					if(node->right)
						node->right->parent = node;
					child->left = node;
					node->parent = child;
					*superparent = child;
					child->parent = parent;
#ifdef AVL_COUNT
					node->count = AVL_CALC_COUNT(node);
					child->count = AVL_CALC_COUNT(child);
#endif
#ifdef AVL_DEPTH
					node->depth = AVL_CALC_DEPTH(node);
					child->depth = AVL_CALC_DEPTH(child);
#endif
				} else {
					// RL型平衡旋转, 1. node的右子节点的左子节点 向右上做一次顺时针旋转,得到RR型
					//               2. node的右子节点向左上做一次逆时针旋转。
					// 这里的实现是直接根据2次旋转的结果进行赋值
					gchild = child->left;
					
					node->right = gchild->left;
					if(node->right)
						node->right->parent = node;
					
					child->left = gchild->right;
					if(child->left)
						child->left->parent = child;
					
					gchild->left = node;
					if(gchild->left)
						gchild->left->parent = gchild;
					
					gchild->right = child;
					if(gchild->right)
						gchild->right->parent = gchild;

					*superparent = gchild;
					gchild->parent = parent;
#ifdef AVL_COUNT
					node->count = AVL_CALC_COUNT(node);
					child->count = AVL_CALC_COUNT(child);
					gchild->count = AVL_CALC_COUNT(gchild);
#endif
#ifdef AVL_DEPTH
					node->depth = AVL_CALC_DEPTH(node);
					child->depth = AVL_CALC_DEPTH(child);
					gchild->depth = AVL_CALC_DEPTH(gchild);
#endif
				}
				break;

			default:
#ifdef AVL_COUNT
				node->count = AVL_CALC_COUNT(node);
#endif
#ifdef AVL_DEPTH
				node->depth = AVL_CALC_DEPTH(node);
#endif
		}
		node = parent;
	}
}

avl_node_t *avl_alloc_node(avl_tree_t *tree)
{
	avl_node_t * node = NULL;
	node = (avl_node_t *)malloc(sizeof(avl_node_t));
	if( node != NULL )
		memset(node,0x00,sizeof(avl_node_t));
	return node;
}

void avl_free_node(avl_tree_t *tree, avl_node_t *node)
{
	if(node->data && tree->del)
		tree->del(node->data);
	free((void*)node);
}

void avl_delete_node(avl_tree_t *tree, avl_node_t *node)
{
	avl_unlink_node(tree, node);
	avl_free_node(tree, node);
}

avl_tree_t *avl_alloc_tree(avl_compare_t compare, avl_action_t del)
{
	avl_tree_t *tree;

	tree = (avl_tree_t *)malloc(sizeof(avl_tree_t));
    if (tree != NULL)
	{
		tree->head = tree->tail = tree->root = NULL;
        tree->compare = compare;
        tree->del = del;
    }
	return tree;
}

void avl_free_tree(avl_tree_t *tree)
{
	free((void*)tree);
}

/* Fast tree cleanup */
void avl_delete_tree(avl_tree_t *tree)
{
	avl_node_t *node = NULL;
    avl_node_t *next = NULL;

	for(node = tree->root; node; node = next) {
		next = node->next;
		avl_free_node(tree, node);
	}

	avl_free_tree(tree);
}

void avl_delete_foreach_tree( avl_tree_t* tree, avl_action_args_t action, void* args )
{
	avl_node_t *node,*next;
	for(node = tree->head; node; node = next){
		next = node->next;
		action(node->data, args);
		avl_free_node( tree, node );
	}
	avl_free_tree(tree);
}

/* Searching */
void *avl_search(const avl_tree_t *tree, const void *data)
{
	avl_node_t *node;

	node = avl_search_node(tree, data);

	return node ? node->data : NULL;
}

/* Insert */
avl_node_t *avl_insert(avl_tree_t *tree, void *data)
{
	avl_node_t *closest, *new;
	int result;

	if(!tree->root) {
		new = avl_alloc_node(tree);
        if (new == NULL) return NULL;
		new->data = data;
		avl_insert_top(tree, new);
	} else {
		closest = avl_search_closest_node(tree, data, &result);

		switch (result) {
			case -1:
				new = avl_alloc_node(tree);
                if (new == NULL) return NULL;
				new->data = data;
				avl_insert_before(tree, closest, new);
				break;

			case 1:
				new = avl_alloc_node(tree);
                if (new == NULL) return NULL;
				new->data = data;
				avl_insert_after(tree, closest, new);
				break;

			default:
				return NULL;
		}
	}

#if 0	/* this is not currect */
#ifdef AVL_COUNT
	new->count = 1;
#endif
#ifdef AVL_DEPTH
	new->depth = 1;
#endif
#endif

	return new;
}

avl_node_t *avl_insert_search( avl_tree_t* tree, void* data, int* flag )
{
	avl_node_t *closest, *new;
	int result;

	if(!tree->root) {
		new = avl_alloc_node(tree);
        if (new == NULL) return NULL;
		new->data = data;
		avl_insert_top(tree, new);
		result = 1;
	} else {
		closest = avl_search_closest_node(tree, data, &result);

		switch (result) {
			case -1:
				new = avl_alloc_node(tree);
                if (new == NULL) return NULL;
				new->data = data;
				avl_insert_before(tree, closest, new);
				result = 1;
				break;

			case 1:
				new = avl_alloc_node(tree);
                if (new == NULL) return NULL;
				new->data = data;
				avl_insert_after(tree, closest, new);
				result = 1;
				break;

			default:
				new = closest;
				result = 0;
				return NULL;
		}
	}
#if  0 /* this is not currect */
#ifdef AVL_COUNT
	new->count = 1;
#endif
#ifdef AVL_DEPTH
	new->depth = 1;
#endif
#endif
	if ( flag )
		*flag = result;
	return new;
}

/* Delete */
void avl_delete(avl_tree_t *tree, void *data)
{
	avl_node_t *node;

	node = avl_search_node(tree, data);

	if(node)
		avl_delete_node(tree, node);
}

void avl_foreach(const avl_tree_t *tree, avl_action_t action)
{
	avl_node_t *node,*next;
	for(node = tree->head; node; node = next){
		next = node->next;
		action(node->data);
	}
}

void *avl_search_closest(const avl_tree_t *tree, const void *data, int *result)
{
	avl_node_t *node = NULL;

	node = avl_search_closest_node(tree, data, result);

	return node ? node->data : NULL;
}

void *avl_search_closest_smaller(const avl_tree_t *tree, const void *data)
{
	avl_node_t *node = NULL;

	node = avl_search_closest_smaller_node(tree, data);

	return node ? node->data : NULL;
}

void *avl_search_closest_greater(const avl_tree_t *tree, const void *data)
{
	avl_node_t *node = NULL;

	node = avl_search_closest_greater_node(tree, data);

	return node ? node->data : NULL;
}

avl_node_t *avl_search_node(const avl_tree_t *tree, const void *data)
{
	avl_node_t *node = NULL;
	int result = 0;

	node = avl_search_closest_node(tree, data, &result);

	return result ? NULL : node;
}

/*
 查找离data最近节点。
 返回值:
 	NULL        : 没有插入任何数据
 	!NULL       : 
 	   result 0 : 返回和data相等的节点
 	          -1: 返回比data大的最近节点
 	          1 : 返回比data小的最近节点
*/
avl_node_t *avl_search_closest_node(const avl_tree_t *tree, const void *data,
									int *result)
{
	avl_node_t *node = NULL;
	int c = 0;

	node = tree->root;

	if(!node) {
		if(result)
			*result = 0;
		return NULL;
	}
	for(;;) {
		c = tree->compare(data, node->data);

		if(c < 0) {
			if(node->left)
				node = node->left;
			else {
				if(result)
					*result = -1;
				break;
			}
		} else if(c > 0) {
			if(node->right)
				node = node->right;
			else {
				if(result)
					*result = 1;
				break;
			}
		} else {
			if(result)
				*result = 0;
			break;
		}
	}

	return node;
}

avl_node_t *avl_search_closest_smaller_node(const avl_tree_t *tree,
											const void *data)
{
	avl_node_t *node = NULL;
	int result = 0;

	node = avl_search_closest_node(tree, data, &result);

	if(result < 0)
		node = node->prev;

	return node;
}

avl_node_t *avl_search_closest_greater_node(const avl_tree_t *tree,
											const void *data)
{
	avl_node_t *node = NULL;
	int result = 0;

	node = avl_search_closest_node(tree, data, &result);

	if(result > 0)
		node = node->next;

	return node;
}

/* Insertion and deletion */
avl_node_t *avl_insert_node(avl_tree_t *tree, avl_node_t *node)
{
	avl_node_t *closest = NULL;
	int result = 0;

	if(!tree->root)
		avl_insert_top(tree, node);
	else {
		closest = avl_search_closest_node(tree, node->data, &result);

		switch (result) {
			case -1:
				avl_insert_before(tree, closest, node);
				break;

			case 1:
				avl_insert_after(tree, closest, node);
				break;

			case 0:
				return NULL;
		}
	}

#if 0	/* this is not currect */
#ifdef AVL_COUNT
	node->count = 1;
#endif
#ifdef AVL_DEPTH
	node->depth = 1;
#endif
#endif

	return node;
}

void avl_insert_top(avl_tree_t *tree, avl_node_t *node)
{
	node->prev = node->next = node->parent = NULL;
	tree->head = tree->tail = tree->root = node;
}

void avl_insert_before(avl_tree_t *tree, avl_node_t *before,
					   avl_node_t *node)
{
	/* edit by dulm : before must != NULL
	if(!before) {
		if(tree->tail)
			avl_insert_after(tree, tree->tail, node);
		else
			avl_insert_top(tree, node);
		return;
	}*/

	node->next = before;
	node->parent = before;
	node->prev = before->prev;

	/* edit by dulm : before->left must == NULL
	if(before->left) {
		avl_insert_after(tree, before->prev, node);
		return;
	}*/

	if(before->prev)
		before->prev->next = node;
	else
		tree->head = node;

	before->prev = node;
	before->left = node;

	avl_rebalance(tree, before);
}

void avl_insert_after(avl_tree_t *tree, avl_node_t *after, avl_node_t *node)
{
	/* edit by dulm : after must != NULL
	if(!after) {
		if(tree->head)
			avl_insert_before(tree, tree->head, node);
		else
			avl_insert_top(tree, node);
		return;
	}*/

	/* edit by dulm : after->right must == NULL
	if(after->right) {
		avl_insert_before(tree, after->next, node);
		return;
	}*/

	node->prev = after;
	node->parent = after;
	node->next = after->next;

	if(after->next)
		after->next->prev = node;
	else
		tree->tail = node;

	after->next = node;
	after->right = node;

	avl_rebalance(tree, after);
}

avl_node_t *avl_unlink(avl_tree_t *tree, void *data)
{
	avl_node_t *node = NULL;

	node = avl_search_node(tree, data);

	if(node)
		avl_unlink_node(tree, node);

	return node;
}

void avl_unlink_node(avl_tree_t *tree, avl_node_t *node)
{
	avl_node_t *parent = NULL;
	avl_node_t **superparent = NULL;
	avl_node_t *subst  = NULL;
    avl_node_t *left   = NULL;
    avl_node_t *right  = NULL;
	avl_node_t *balnode = NULL;

	// 在排序的链表中，把当前节点删除
	if(node->prev)
		node->prev->next = node->next;
	else
		tree->head = node->next;
	if(node->next)
		node->next->prev = node->prev;
	else
		tree->tail = node->prev;

	parent = node->parent;

	superparent =
		parent ? node ==
		parent->left ? &parent->left : &parent->right : &tree->root;

	left = node->left;
	right = node->right;
	if(!left) {	// 删除node只包含右子树，则该右子树的高度 <= 1, 删除该node只需要用右节点替换掉node
		*superparent = right;

		if(right)
			right->parent = parent;

		balnode = parent;
	} else if(!right) {	// 删除node只包含左子树，并且左节点!=NULL, 高度<=1, 删除该node只需要用左节点替换掉node
		*superparent = left;
		left->parent = parent;
		balnode = parent;
	} else { // node有左子树和右子树，找到比node小的最大节点(prev)，替换掉node，并做好树结构
		subst = node->prev;

		if(subst == left) {
			balnode = subst;
		} else {
			balnode = subst->parent;
			balnode->right = subst->left;

			if(balnode->right)
				balnode->right->parent = balnode;

			subst->left = left;
			left->parent = subst;
		}

		subst->right = right;
		subst->parent = parent;
		right->parent = subst;
		*superparent = subst;
	}

	avl_rebalance(tree, balnode);

	node->next = node->prev = node->parent = node->left = node->right = NULL;

#ifdef AVL_COUNT
	node->count = 0;
#endif
#ifdef AVL_DEPTH
	node->depth = 0;
#endif
}

/* Tree walking */
void avl_foreach_node(const avl_tree_t *tree, avl_action_t action)
{
	avl_node_t *node = NULL;
	avl_node_t *next = NULL;

	for(node = tree->head; node; node = next)
	{
		next = node->next;
		action(node);
	}
}

/* Indexing */

#ifdef AVL_COUNT
unsigned int avl_count(const avl_tree_t *tree)
{
	return AVL_NODE_COUNT(tree->root);
}

avl_node_t *avl_get_node(const avl_tree_t *tree, unsigned int index)
{
	avl_node_t *node = NULL;
	unsigned int c = 0;

	node = tree->root;

	while(node)
	{
		c = AVL_L_COUNT(node);

		if ( index < c )
		{
			node = node->left;
		}
		else if ( index > c )
		{
			node = node->right;
			index -= c + 1;
		}
		else
		{
			return node;
		}
	}

	return NULL;
}

unsigned int avl_index(const avl_node_t *node)
{
	avl_node_t *next = NULL;
	unsigned int index = 0;

	index = AVL_L_COUNT(node);

	while((next = node->parent))
	{
		if(node == next->right)
			index += AVL_L_COUNT(next) + 1;
		node = next;
	}

	return index;
}
#endif
#ifdef AVL_DEPTH
unsigned int avl_depth(const avl_tree_t *tree)
{
	return AVL_NODE_DEPTH(tree->root);
}
#endif

