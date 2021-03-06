/*
    avl_tree.h -- header file for avl_tree.c
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

    $Id: avl_tree.h,v 1.1.2.10 2003/07/24 12:08:15 guus Exp $
*/


#ifndef __AVL_TREE_H__
#define __AVL_TREE_H__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef AVL_DEPTH
	#ifndef AVL_COUNT
		#define AVL_DEPTH
	#endif
#endif

typedef struct avl_node_t {

	/* Linked list part */

	struct avl_node_t *next;
	struct avl_node_t *prev;

	/* Tree part */

	struct avl_node_t *parent;
	struct avl_node_t *left;
	struct avl_node_t *right;

#ifdef AVL_COUNT
	unsigned int count;
#endif
#ifdef AVL_DEPTH
	unsigned char depth;
#endif

	/* Payload */

	void *data;

} avl_node_t;

typedef int (*avl_compare_t)(const void *, const void *);
typedef void (*avl_action_t)(const void *);
typedef void (*avl_action_node_t)(const avl_node_t *);
typedef void (*avl_action_args_t)( const void*, const void* );

typedef struct avl_tree_t {

	/* Linked list part */

	avl_node_t *head;
	avl_node_t *tail;

	/* Tree part */

	avl_node_t *root;

	avl_compare_t compare;
	avl_action_t del;

} avl_tree_t;

/* (De)constructors */

#ifdef __cplusplus
extern "C" {
#endif


extern avl_tree_t *avl_alloc_tree(avl_compare_t, avl_action_t);
extern void avl_delete_tree(avl_tree_t *);

extern void avl_delete_foreach_tree( avl_tree_t *, avl_action_args_t, void* );

extern void *avl_search(const avl_tree_t *, const void *);
extern avl_node_t *avl_insert(avl_tree_t *, void *);

extern avl_node_t *avl_insert_search( avl_tree_t*, void*, int* );

extern void  avl_delete(avl_tree_t *, void *);


extern void avl_foreach(const avl_tree_t *, avl_action_t);

extern avl_node_t *avl_alloc_node(avl_tree_t *);
extern void avl_free_node(avl_tree_t *tree, avl_node_t *);

/* Insertion and deletion */
extern avl_node_t *avl_insert_node(avl_tree_t *, avl_node_t *);

extern void avl_insert_top(avl_tree_t *, avl_node_t *);
extern void avl_insert_before(avl_tree_t *, avl_node_t *, avl_node_t *);
extern void avl_insert_after(avl_tree_t *, avl_node_t *, avl_node_t *);

extern avl_node_t *avl_unlink(avl_tree_t *, void *);
extern void avl_unlink_node(avl_tree_t *tree, avl_node_t *);
extern void avl_delete_node(avl_tree_t *, avl_node_t *);

/* Searching */
extern void *avl_search_closest(const avl_tree_t *, const void *, int *);
extern void *avl_search_closest_smaller(const avl_tree_t *, const void *);
extern void *avl_search_closest_greater(const avl_tree_t *, const void *);

extern avl_node_t *avl_search_node(const avl_tree_t *, const void *);
extern avl_node_t *avl_search_closest_node(const avl_tree_t *, const void *, int *);
extern avl_node_t *avl_search_closest_smaller_node(const avl_tree_t *, const void *);
extern avl_node_t *avl_search_closest_greater_node(const avl_tree_t *, const void *);

/* Tree walking */
extern void avl_foreach_node(const avl_tree_t *, avl_action_t);

/* Indexing */

#ifdef AVL_COUNT
extern unsigned int avl_count(const avl_tree_t *);
extern avl_node_t *avl_get_node(const avl_tree_t *, unsigned int);
extern unsigned int avl_index(const avl_node_t *);
#endif
#ifdef AVL_DEPTH
extern unsigned int avl_depth(const avl_tree_t *);
#endif

#ifdef __cplusplus
}
#endif

#endif							/* __AVL_TREE_H__ */
