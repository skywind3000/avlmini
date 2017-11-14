#include "avlmini.c"
#include "test/linux_rbtree.c"
#include "test_avl.h"



//---------------------------------------------------------------------
// random 
//---------------------------------------------------------------------
static void benchmark(const char *text, int mode, int count)
{
	int *keys;
	struct avl_node **avl_nodes = NULL;
	struct rb_node **rb_nodes = NULL;
	struct avl_root avl_root;
	struct rb_root rb_root;
	unsigned int ts, total = 0;
	int i, missing = 0;

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
			/* avl_test_validate(&avl_root); */
		}
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			struct rb_node *dup;
			struct rb_node *node = rb_nodes[i];
			rb_node_add(&rb_root, node, rb_node_compare, dup);
			assert(dup == NULL);
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

	// test search
	if (mode == 0) {
		for (i = 0; i < count; i++) {
			int key = keys[count - 1 - i];
			struct MyNode *result;
			struct avl_node *res;
			struct MyNode dummy;
			dummy.key = key;
			avl_node_find(&avl_root, &dummy.node, avl_node_compare, res);
			result = AVL_ENTRY(res, struct MyNode, node);
			assert(result);
			assert(result->key == key);
		}
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			int key = keys[count - 1 - i];
			struct RbNode *result;
			struct rb_node *res;
			struct RbNode dummy;
			dummy.key = key;
			rb_node_find(&rb_root, &dummy.node, rb_node_compare, res);
			result = rb_entry(res, struct RbNode, node);
			assert(result->key == key);
		}
	}

	ts = gettime() - ts;
	total += ts;
	printf("search time: %dms error=%d\n", (int)ts, missing);

	sleepms(200);
	ts = gettime();

	if (mode == 0) {
		for (i = 0; i < count; i++) {
			struct avl_node *node = avl_root.node;
			assert(node);
			avl_node_erase(node, &avl_root);
			/* avl_test_validate(&avl_root); */
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
	benchmark("linux rbtree", 1, COUNT2);
	benchmark("avlmini", 0, COUNT2);
	benchmark("linux rbtree", 1, COUNT3);
	benchmark("avlmini", 0, COUNT3);
}

void test3()
{
	benchmark("avlmini", 0, 1000);
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
sizeof=16/16
linux rbtree with 10000000 nodes:
insert time: 2187ms, height=33
search time: 1266ms error=0
delete time: 469ms
total: 3922ms

avlmini with 10000000 nodes:
insert time: 2141ms, height=27
search time: 1234ms error=0
delete time: 515ms
total: 3890ms

linux rbtree with 1000000 nodes:
insert time: 187ms, height=27
search time: 125ms error=0
delete time: 39ms
total: 343ms

avlmini with 1000000 nodes:
insert time: 188ms, height=24
search time: 109ms error=0
delete time: 48ms
total: 360ms

linux rbtree with 100000 nodes:
insert time: 15ms, height=20
search time: 0ms error=0
delete time: 0ms
total: 15ms

avlmini with 100000 nodes:
insert time: 16ms, height=20
search time: 15ms error=0
delete time: 0ms
total: 31ms
*/


