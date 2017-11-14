#ifndef _TEST_AVL_H_
#define _TEST_AVL_H_

#include "avlmini.h"
#include "test/test_linux_rb.h"
#include "test/printt.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#if (defined(_WIN32) || defined(WIN32))
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#elif defined(__unix)
#include <sys/time.h>
#include <unistd.h>
#else
#error it can only be compiled under windows or unix
#endif


/* gettime */
static inline unsigned int gettime()
{
	#if (defined(_WIN32) || defined(WIN32))
	return timeGetTime();
	#else
	static struct timezone tz={ 0,0 };
	struct timeval time;
	gettimeofday(&time,&tz);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
	#endif
}

static inline void sleepms(unsigned int millisec)
{
#if defined(_WIN32) || defined(WIN32)
	Sleep(millisec);
#else
	usleep(millisec * 1000);
#endif
}


struct MyNode
{
	struct avl_node node;
	int key;
	int val;
};

#define avl_key(node) (((struct MyNode*)(node))->key)

static inline struct MyNode *avl_node_new(int key)
{
	struct MyNode *node = (struct MyNode*)malloc(sizeof(struct MyNode));
	node->key = key;
	return node;
}

static inline int avl_node_compare(const void *n1, const void *n2)
{
	struct MyNode *x = (struct MyNode*)n1;
	struct MyNode *y = (struct MyNode*)n2;
	return x->key - y->key;
}

static inline int avl_test_bst(struct avl_root *tree)
{
	struct avl_node *node = avl_node_first(tree);
	int value;
	if (node == NULL) return 0;
	value = avl_key(node);
	node = avl_node_next(node);
	for (; node; node = avl_node_next(node)) {
		int x = avl_key(node);
		if (x <= value) {
			printf("test failed\n");
			return -1;
		}
		value = x;
	}
	return 0;
}

static int avl_test_height(struct avl_node *node, int *error)
{
	if (node == NULL) {
		return 0;
	}
	else {
		int h0 = avl_test_height(node->left, error);
		int h1 = avl_test_height(node->right, error);
		int mh = (h0 > h1)? h0 : h1;
		int dh = (h0 > h1)? h0 - h1 : h1 - h0;
		if (node->height != mh + 1) {
			printf("height mismatch %d <-> %d\n",node->height, mh + 1);
			error[0]++;
			assert(0);
			return 0;
		}
		if (dh >= 2) {
			printf("over balance %d/%d\n", h0, h1);
			error[0]++;
			assert(0);
			return 0;
		}
		return mh + 1;
	}
}

static int avl_test_father(struct avl_node *node, int *error)
{
	if (node == NULL) {
		return 0;
	}
	else {
		if (node->left) {
			if (node->left->parent != node) {
				printf("n%d.left.parent error\n", avl_key(node));
				if (node->left->parent) {
					printf("current parent=%d\n", avl_key(node->left->parent));
				}
				if (error) error[0]++;
				assert(0);
				return 0;
			}
		}
		if (node->right) {
			if (node->right->parent != node) {
				printf("n%d.right.parent error\n", avl_key(node));
				if (node->right->parent) {
					printf("current parent=%d\n", avl_key(node->right->parent));
				}
				if (error) error[0]++;
				assert(0);
				return 0;
			}
		}
		avl_test_father(node->left, error);
		avl_test_father(node->right, error);
	}
	return 0;
}

static inline int avl_test_validate(struct avl_root *tree)
{
	int error = 0;
	/* printf("avl validate: "); */
	error = avl_test_bst(tree);
	if (error) {
		return error;
	}
	avl_test_father(tree->node, &error);
	if (error) {
		return error;
	}
	avl_test_height(tree->node, &error);
	if (error) {
		return error;
	}
	/* printf("ok\n"); */
	return error;
}


#define RANDOM(n) (xrand() % (n))
static unsigned int xseed = 0x11223344;
static inline unsigned int xrand(void) {
	return (((xseed = xseed * 214013L + 2531011L) >> 16) & 0x7fffffff);
}

// generate keys
static inline void random_keys(int *keys, int count, int seed)
{
	int save_seed = xseed;
	int *array = (int*)malloc(sizeof(int) * count);
	int length = count, i;
	for (i = 0; i < count; i++) {
		array[i] = i;
	}
	for (i = 0; i < length; i++) {
		int pos = xrand() % count;
		int key = array[pos];
		keys[i] = key;
		array[pos] = array[--count];
	}
	free(array);
	xseed = save_seed;
}


static int avl_tree_height(struct avl_node *node)
{
	if (node == NULL) 
		return 0;
	else if (node->left == NULL && node->right == NULL) 
		return 1;
	else
		return _int_max(avl_tree_height(node->left),
				avl_tree_height(node->right)) + 1;
}

#endif



