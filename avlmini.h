/*********************************************************************
 *
 * avlmini.h - fast as linux's rbtree, but much smaller
 *
 * NOTE:
 * for more information, please see the readme file
 *
 *********************************************************************/
#ifndef _AVLMINI_H__
#define _AVLMINI_H__


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef HAVE_NOT_STDDEF_H
#include <stddef.h>
#endif


/*====================================================================*/
/* GLOBAL MACROS                                                      */
/*====================================================================*/
#ifndef INLINE
#if defined(__GNUC__)

#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
#define INLINE         __inline__ __attribute__((always_inline))
#else
#define INLINE         __inline__
#endif

#elif (defined(_MSC_VER) || defined(__WATCOMC__))
#define INLINE __inline
#else
#define INLINE 
#endif
#endif

#if (!defined(__cplusplus)) && (!defined(inline))
#define inline INLINE
#endif

/* you can change this by config.h or predefined macro */
#ifndef ASSERTION
#define ASSERTION(x) ((void)0)
#endif


/*====================================================================*/
/* avl_node - avl binary search tree                                  */
/*====================================================================*/
struct avl_node
{
	struct avl_node *left;
	struct avl_node *right;
	struct avl_node *parent;    /* pointing to node itself for empty node */
	int height;                 /* equals to 1 + max height in childs */
};

struct avl_root
{
	struct avl_node *node;		/* root node */
};


/*--------------------------------------------------------------------*/
/* NODE MACROS                                                        */
/*--------------------------------------------------------------------*/
#define AVL_LEFT    0        /* left child index */
#define AVL_RIGHT   1        /* right child index */

#define AVL_OFFSET(TYPE, MEMBER)    ((size_t) &((TYPE *)0)->MEMBER)

#define AVL_NODE2DATA(n, o)    ((void *)((size_t)(n) - (o)))
#define AVL_DATA2NODE(d, o)    ((struct avl_node*)((size_t)(d) + (o)))

#define AVL_ENTRY(ptr, type, member) \
	((type*)AVL_NODE2DATA(ptr, AVL_OFFSET(type, member)))

#define avl_node_init(node) do { ((node)->parent) = (node); } while (0)
#define avl_node_empty(node) ((node)->parent == (node))

#define AVL_LEFT_HEIGHT(node) (((node)->left)? ((node)->left)->height : 0)
#define AVL_RIGHT_HEIGHT(node) (((node)->right)? ((node)->right)->height : 0)


#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------*/
/* binary search tree - node manipulation                             */
/*--------------------------------------------------------------------*/

struct avl_node *avl_node_first(struct avl_root *root);
struct avl_node *avl_node_last(struct avl_root *root);
struct avl_node *avl_node_next(struct avl_node *node);
struct avl_node *avl_node_prev(struct avl_node *node);

void avl_node_replace(struct avl_node *victim, struct avl_node *newnode,
		struct avl_root *root);

static inline void avl_node_link(struct avl_node *node, struct avl_node *parent,
		struct avl_node **avl_link) {
	node->parent = parent;
	node->height = 0;
	node->left = node->right = NULL;
	avl_link[0] = node;
}

/* avl insert rebalance and erase */
void avl_node_post_insert(struct avl_node *node, struct avl_root *root);
void avl_node_erase(struct avl_node *node, struct avl_root *root);

/* tear down the whole tree */
struct avl_node* avl_node_tear(struct avl_root *root, struct avl_node **next);


/*--------------------------------------------------------------------*/
/* avl node templates                                                 */
/*--------------------------------------------------------------------*/

#define avl_node_find(root, what, compare_fn, res_node) do {\
		struct avl_node *__n = (root)->node; \
		(res_node) = NULL; \
		while (__n) { \
			int __hr = (compare_fn)(what, __n); \
			if (__hr == 0) { (res_node) = __n; break; } \
			else if (__hr < 0) { __n = __n->left; } \
			else { __n = __n->right; } \
		} \
	}   while (0)


#define avl_node_add(root, newnode, compare_fn, duplicate_node) do { \
		struct avl_node **__link = &((root)->node); \
		struct avl_node *__parent = NULL; \
		struct avl_node *__duplicate = NULL; \
		int __hr = 1; \
		while (__link[0]) { \
			__parent = __link[0]; \
			__hr = (compare_fn)(newnode, __parent); \
			if (__hr == 0) { __duplicate = __parent; break; } \
			else if (__hr < 0) { __link = &(__parent->left); } \
			else { __link = &(__parent->right); } \
		} \
		(duplicate_node) = __duplicate; \
		if (__duplicate == NULL) { \
			avl_node_link(newnode, __parent, __link); \
			avl_node_post_insert(newnode, root); \
		} \
	}   while (0)


/*====================================================================*/
/* avl_tree - easy interface                                          */
/*====================================================================*/

struct avl_tree
{
	struct avl_root root;		/* avl root */
	size_t offset;				/* node offset in user data structure */
	size_t size;                /* size of user data structure */
	size_t count;				/* node count */
	/* returns 0 for equal, -1 for n1 < n2, 1 for n1 > n2 */
	int (*compare)(const void *n1, const void *n2);
};


/* initialize avltree, use AVL_OFFSET(type, member) for "offset"
 * eg:
 *     avl_tree_init(&mytree, mystruct_compare,
 *          sizeof(struct mystruct_t), 
 *          AVL_OFFSET(struct mystruct_t, node));
 */
void avl_tree_init(struct avl_tree *tree,
		int (*compare)(const void*, const void*), size_t size, size_t offset);

void *avl_tree_first(struct avl_tree *tree);
void *avl_tree_last(struct avl_tree *tree);
void *avl_tree_next(struct avl_tree *tree, void *data);
void *avl_tree_prev(struct avl_tree *tree, void *data);

/* require a temporary user structure (data) which contains the key */
void *avl_tree_find(struct avl_tree *tree, const void *data);
void *avl_tree_nearest(struct avl_tree *tree, const void *data);

/* returns NULL for success, otherwise returns conflict node with same key */
void *avl_tree_add(struct avl_tree *tree, void *data);

void avl_tree_remove(struct avl_tree *tree, void *data);
void avl_tree_replace(struct avl_tree *tree, void *victim, void *newdata);

void avl_tree_clear(struct avl_tree *tree, void (*destroy)(void *data));




#ifdef __cplusplus
}
#endif


#endif


