#include "avlmini.h"


/*====================================================================*/
/* Binary Search Tree                                                 */
/*====================================================================*/

/* LEFT is 0: walk towards left, and 1 for right */
static inline struct avl_node *_avl_node_walk(struct avl_node *node, int LEFT)
{
	int RIGHT = LEFT ^ 1;
	if (node == NULL) return NULL;
	
	if (node->child[LEFT]) {
		node = node->child[LEFT];
		while (node->child[RIGHT]) 
			node = node->child[RIGHT];
	}
	else {
		while (1) {
			struct avl_node *last = node;
			node = node->parent;
			if (node == NULL) break;
			if (node->child[RIGHT] == last) break;
		}
	}

	return node;
}

/* LEFT is 0: left head, and 1 for right head */
static inline struct avl_node *_avl_node_head(struct avl_root *root, int LEFT)
{
	struct avl_node *node = root->node;
	if (node == NULL) return NULL;
	while (node->child[LEFT]) 
		node = node->child[LEFT];
	return node;
}

struct avl_node *avl_node_first(struct avl_root *root)
{
	return _avl_node_head(root, 0);
}

struct avl_node *avl_node_last(struct avl_root *root)
{
	return _avl_node_head(root, 1);
}

struct avl_node *avl_node_next(struct avl_node *node)
{
	return _avl_node_walk(node, 1);
}

struct avl_node *avl_node_prev(struct avl_node *node)
{
	return _avl_node_walk(node, 0);
}

static inline void 
_avl_child_replace(struct avl_node *oldnode, struct avl_node *newnode, 
		struct avl_node *parent, struct avl_root *root) 
{
	if (parent)
		parent->child[(parent->child[1] == oldnode)? 1 : 0] = newnode;
	else 
		root->node = newnode;
}

#if 1
int avl_node_rotate_times = 0;
#endif

static inline struct avl_node *
_avl_node_rotate(struct avl_node *node, struct avl_root *root, int LEFT)
{
	int RIGHT = LEFT ^ 1;
	struct avl_node *right = node->child[RIGHT];
	struct avl_node *parent = node->parent;
	node->child[RIGHT] = right->child[LEFT];
#if 0
	avl_node_rotate_times++;
#endif
	ASSERTION(node && right);
	if (right->child[LEFT]) 
		right->child[LEFT]->parent = node;
	right->child[LEFT] = node;
	right->parent = parent;
	_avl_child_replace(node, right, parent, root);
	node->parent = right;
	return right;
}


void avl_node_replace(struct avl_node *victim, struct avl_node *newnode,
		struct avl_root *root)
{
	struct avl_node *parent = victim->parent;
	_avl_child_replace(victim, newnode, parent, root);
	if (victim->child[0]) victim->child[0]->parent = newnode;
	if (victim->child[1]) victim->child[1]->parent = newnode;
	newnode->child[0] = victim->child[0];
	newnode->child[1] = victim->child[1];
	newnode->parent = victim->parent;
	newnode->height = victim->height;
}


/*--------------------------------------------------------------------*/
/* avl - node manipulation                                            */
/*--------------------------------------------------------------------*/

static inline int AVL_MAX(int x, int y) 
{
	return (x < y)? y : x;
}

static inline void
_avl_node_height_update(struct avl_node *node)
{
	int h0 = AVL_CHILD_HEIGHT(node, 0);
	int h1 = AVL_CHILD_HEIGHT(node, 1);
	node->height = AVL_MAX(h0, h1) + 1;
}

static inline struct avl_node *
_avl_node_fix(struct avl_node *node, struct avl_root *root, int LEFT)
{
	int RIGHT = 1 - LEFT;
	struct avl_node *right = node->child[RIGHT];
	int rh0, rh1;
	ASSERTION(right);
	rh0 = AVL_CHILD_HEIGHT(right, LEFT);
	rh1 = AVL_CHILD_HEIGHT(right, RIGHT);
	if (rh0 > rh1) {
		right = _avl_node_rotate(right, root, RIGHT);
		_avl_node_height_update(right->child[RIGHT]);
		_avl_node_height_update(right);
		/* _avl_node_height_update(node); */
	}
	node = _avl_node_rotate(node, root, LEFT);
	_avl_node_height_update(node->child[LEFT]);
	_avl_node_height_update(node);
	return node;
}

static inline void 
_avl_node_rebalance(struct avl_node *node, struct avl_root *root)
{
	while (node) {
		int h0 = (int)AVL_CHILD_HEIGHT(node, 0);
		int h1 = (int)AVL_CHILD_HEIGHT(node, 1);
		int height = AVL_MAX(h0, h1) + 1;
		int diff = h0 - h1;
		if (node->height != height) {
			node->height = height;
		}	
		else if (diff >= -1 && diff <= 1) {
			break;
		}
		/* printf("rebalance %d\n", avl_value(node)); */
		if (diff <= -2) {
			node = _avl_node_fix(node, root, 0);
		}
		else if (diff >= 2) {
			node = _avl_node_fix(node, root, 1);
		}
		node = node->parent;
		/* printf("parent %d\n", (!node)? -1 : avl_value(node)); */
	}
}

void avl_node_post_insert(struct avl_node *node, struct avl_root *root)
{
	node->height = 1;
	_avl_node_rebalance(node->parent, root);
}

void avl_node_erase(struct avl_node *node, struct avl_root *root)
{
	struct avl_node *child, *parent;
	ASSERTION(node);
	if (node->child[0] && node->child[1]) {
		struct avl_node *old = node;
		struct avl_node *left;
		node = node->child[AVL_RIGHT];
		while ((left = node->child[AVL_LEFT]) != NULL)
			node = left;
		child = node->child[AVL_RIGHT];
		parent = node->parent;
		if (child) {
			child->parent = parent;
		}
		_avl_child_replace(node, child, parent, root);
		if (node->parent == old)
			parent = node;
		node->child[0] = old->child[0];
		node->child[1] = old->child[1];
		node->parent = old->parent;
		node->height = old->height;
		_avl_child_replace(old, node, old->parent, root);
		ASSERTION(old->child[AVL_LEFT]);
		old->child[AVL_LEFT]->parent = node;
		if (old->child[AVL_RIGHT]) {
			old->child[AVL_RIGHT]->parent = node;
		}
	}
	else {
		child = node->child[(node->child[0] == NULL)? 1 : 0];
		parent = node->parent;
		_avl_child_replace(node, child, parent, root);
		if (child) {
			child->parent = parent;
		}
	}
	if (parent) {
		_avl_node_rebalance(parent, root);
	}
}


