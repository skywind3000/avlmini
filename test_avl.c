#include "avlmini.c"
#include "test/linux_rbtree.c"
#include "test_avl.h"



//---------------------------------------------------------------------
// random 
//---------------------------------------------------------------------
struct MyNode* avl_mother(struct avl_root *root, int key)
{
	struct avl_node *node = (struct avl_node*)(void*)root->node;
	while (node) {
		struct MyNode *data = AVL_ENTRY(node, struct MyNode, node);
		if (key == data->key) {
			return data;
		}
		else if (key < data->key) {
			node = node->child[0];
		}
		else {
			node = node->child[1];
		}
	}
	return NULL;
}

struct RbNode* rb_mother(struct rb_root *root, int key)
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

static void benchmark(const char *text, int mode, int count)
{
	int *keys;
	struct avl_node **avl_nodes = NULL;
	struct rb_node **rb_nodes = NULL;
	struct avl_root avl_root;
	struct rb_root rb_root;
	unsigned int ts, total = 0;
	int i;

	keys = (int*)malloc(sizeof(int) * count);
	random_keys(keys, count, 0x11223344);
	if (mode == 0) {
		avl_nodes = (struct avl_node**)malloc(sizeof(void*) * count);
		for (i = 0; i < count; i++) {
			avl_nodes[i] = (struct avl_node*)avl_node_new(keys[i]);
		}
		avl_root.node = NULL;
	}
	else if (mode == 1) {
		rb_nodes = (struct rb_node**)malloc(sizeof(void*) * count);
		for (i = 0; i < count; i++) {
			rb_nodes[i] = (struct rb_node*)rb_node_new(keys[i]);
		}
		rb_root.rb_node = NULL;
	}

	printf("%s with %d nodes:\n", text, count);

	sleepms(400);
	ts = gettime();

	// test insert
	if (mode == 0) {
		for (i = 0; i < count; i++) {
			struct avl_node *dup;
			struct avl_node *node = avl_nodes[i];
			avl_node_add(&avl_root, node, avl_node_compare, dup);
			assert(dup == NULL);
		}
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			struct rb_node *dup;
			struct rb_node *node = rb_nodes[i];
			rb_node_add(&rb_root, node, rb_node_compare, dup);
		}
	}

	ts = gettime() - ts;
	total += ts;
	printf("insert time: %dms", (int)ts);

	if (mode == 0) {
		printf(", height=%d\n", avl_tree_height(avl_root.node));
		avl_test_validate(&avl_root);
		avl_node_first(&avl_root);
	}
	else {
		printf(", height=%d\n", rb_tree_height(rb_root.rb_node));
		rb_first(&rb_root);
	}

	sleepms(200);
	ts = gettime();

	if (mode == 0) {
		for (i = 0; i < count; i++) {
			int key = keys[i];
			struct MyNode *result;
			result = avl_search(&avl_root, key);
			assert(result);
			assert(result->key == key);
		}
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			int key = keys[i];
			struct RbNode *result;
			result = rb_search(&rb_root, key);
			assert(result);
			assert(result->key == key);
		}
	}

	ts = gettime() - ts;
	total += ts;
	printf("search time: %dms\n", (int)ts);

	sleepms(200);
	ts = gettime();

	if (mode == 0) {
		for (i = 0; i < count; i++) {
			struct avl_node *node = avl_root.node;
			assert(node);
			avl_node_erase(node, &avl_root);
		}
		assert(avl_root.node == NULL);
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			struct rb_node *node = rb_root.rb_node;
			assert(node);
			rb_erase(node, &rb_root);
		}
	}

	ts = gettime() - ts;
	total += ts;
	printf("delete time: %dms\n", (int)ts);

	if (avl_nodes) {
		for (i = 0; i < count; i++) 
			free(avl_nodes[i]);
		free(avl_nodes);
	}

	if (rb_nodes) {
		for (i = 0; i < count; i++) 
			free(rb_nodes[i]);
		free(rb_nodes);
	}

	printf("total: %dms\n", (int)total);
	printf("\n");
}

void test1()
{
	int a[100];
	int i;
	random_keys(a, 100, 0x11223344);
	for (i = 0; i < 100; i++) printf("%d\n", a[i]);
}

void test2()
{
#define COUNT    10000000
#define COUNT2   1000000
#define COUNT3   100000
	benchmark("linux rbtree", 1, COUNT);
	benchmark("avlmini", 0, COUNT);
	/* benchmark("linux rbtree", 1, COUNT2); */
	/* benchmark("avlmini", 0, COUNT2); */
	/* benchmark("linux rbtree", 1, COUNT3); */
	/* benchmark("avlmini", 0, COUNT3); */
}

void test3()
{
}

int main(void)
{
#ifdef _WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
	printf("sizeof=%d/%d\n", sizeof(struct avl_node), sizeof(struct rb_node));
	test2();
	return 0;
}


/*
avlmini with 10000000 nodes:
insert time: 2156ms, height=27
search time: 1500ms
delete time: 532ms
total: 4188ms

linux rbtree with 10000000 nodes:
insert time: 2125ms, height=33
search time: 1672ms
delete time: 453ms
total: 4250ms

avlmini with 1000000 nodes:
insert time: 187ms, height=24
search time: 125ms
delete time: 47ms
total: 359ms

linux rbtree with 1000000 nodes:
insert time: 171ms, height=27
search time: 156ms
delete time: 31ms
total: 358ms

avlmini with 100000 nodes:
insert time: 15ms, height=20
search time: 0ms
delete time: 0ms
total: 15ms

linux rbtree with 100000 nodes:
insert time: 15ms, height=20
search time: 16ms
delete time: 0ms
total: 31ms*/


