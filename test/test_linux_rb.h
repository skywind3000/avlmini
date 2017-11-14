#ifndef _TEST_LINUX_RB_H_
#define _TEST_LINUX_RB_H_

#include "linux_rbtree.h"

#include <stdio.h>
#include <stdlib.h>


#define rb_node_find(root, what, compare_fn, res_node) do {\
		struct rb_node *__n = (root)->rb_node; \
		(res_node) = NULL; \
		while (__n) { \
			int __hr = (compare_fn)(what, __n); \
			(res_node) = __n; \
			if (__hr == 0) { (res_node) = __n; break; } \
			else if (__hr < 0) { __n = __n->rb_left; } \
			else { __n = __n->rb_right; } \
		} \
	}   while (0)

#define rb_node_add(root, newnode, compare_fn, duplicate_node) do { \
		struct rb_node **__link = &((root)->rb_node); \
		struct rb_node *__parent = NULL; \
		struct rb_node *__duplicate = NULL; \
		int __hr = 1; \
		while (__link[0]) { \
			__parent = __link[0]; \
			__hr = (compare_fn)(newnode, __parent); \
			if (__hr == 0) { __duplicate = __parent; break; } \
			else if (__hr < 0) { __link = &(__link[0]->rb_left); }\
			else { __link = &(__link[0]->rb_right); }\
		} \
		(duplicate_node) = __duplicate; \
		if (__duplicate == NULL) { \
			rb_link_node(newnode, __parent, __link); \
			rb_insert_color(newnode, root); \
		} \
	}   while (0)



//---------------------------------------------------------------------
// my rbnode
//---------------------------------------------------------------------
struct RbNode
{
	struct rb_node node;
	int key;
};


static inline struct RbNode *rb_node_new(int key)
{
	struct RbNode *node = (struct RbNode*)malloc(sizeof(struct RbNode));
	node->key = key;
	return node;
}

static inline int rb_node_compare(const void *n1, const void *n2)
{
	struct RbNode *x = (struct RbNode*)n1;
	struct RbNode *y = (struct RbNode*)n2;
	return x->key - y->key;
}


#define offset_of(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define container_of(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - offset_of(type, member)) )

static inline int _int_max(int x, int y) { return (x > y)? x : y;  }


static int rb_tree_height(struct rb_node *node)
{
	if (node == NULL) 
		return 0;
	else if (node->rb_left == NULL && node->rb_right == NULL) 
		return 1;
	else
		return _int_max(rb_tree_height(node->rb_left),
				rb_tree_height(node->rb_right)) + 1;
}

static inline struct RbNode *rb_search(struct rb_root *root, int key)
{
	struct rb_node *node = root->rb_node;
	while (node) {
		struct RbNode *data = rb_entry(node, struct RbNode, node);
		if (key == data->key) {
			return data;
		}
		else if (key < data->key) {
			node = node->rb_left;
		}
		else {
			node = node->rb_right;
		}
	}
	return NULL;
}


#endif



