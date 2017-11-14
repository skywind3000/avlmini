#include "avlmini.c"
#include "test/linux_rbtree.c"
#include "test_avl.h"

#include <map>


//---------------------------------------------------------------------
// random 
//---------------------------------------------------------------------
static void benchmark(const char *text, int mode, int count)
{
	int *keys;
	struct avl_root avl_root;
	struct rb_root rb_root;
	unsigned int ts, total = 0;
	int i, missing = 0;
	std::map<int, int> stlmap;

	keys = (int*)malloc(sizeof(int) * count);
	random_keys(keys, count, 0x11223344);
	if (mode == 0) {
		avl_root.node = NULL;
	}
	else if (mode == 1) {
		rb_root.rb_node = NULL;
	}

	printf("%s with %d nodes:\n", text, count);

	sleepms(400);
	ts = gettime();

	// test insert
	if (mode == 0) {
		for (i = 0; i < count; i++) {
			struct avl_node *dup;
			struct MyNode *node = avl_node_new(keys[i]);
			node->val = node->key * 10;
			avl_node_add(&avl_root, &(node->node), avl_node_compare, dup);
			assert(dup == NULL);
		}
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			struct rb_node *dup;
			struct RbNode *node = rb_node_new(keys[i]);
			node->val = node->key * 10;
			rb_node_add(&rb_root, &(node->node), rb_node_compare, dup);
			assert(dup == NULL);
		}
	}
	else if (mode == 2) {
		for (i = 0; i < count; i++) {
			int key = keys[i];
			stlmap[key] = key * 10;
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
	else if (mode == 1) {
		printf(", height=%d\n", rb_tree_height(rb_root.rb_node));
		rb_first(&rb_root);
	}
	else {
		printf("\n");
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
	else if (mode == 2) {
		for (i = 0; i < count; i++) {
			int key = keys[count - 1 - i];
			std::map<int, int>::iterator it = stlmap.find(key);
			assert(it != stlmap.end());
		}
	}

	ts = gettime() - ts;
	total += ts;
	printf("search time: %dms error=%d\n", (int)ts, missing);

	sleepms(200);
	ts = gettime();

	if (mode == 0) {
		for (i = 0; i < count; i++) {
			struct avl_node *node = avl_node_first(&avl_root);
			assert(node);
			avl_node_erase(node, &avl_root);
			/* free(node); */
			/* avl_test_validate(&avl_root); */
		}
		assert(avl_root.node == NULL);
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			struct rb_node *node = rb_first(&rb_root);
			assert(node);
			rb_erase(node, &rb_root);
			/* free(node); */
		}
	}
	else if (mode == 2) {
		for (i = 0; i < count; i++) {
			std::map<int, int>::iterator it = stlmap.begin();
			assert(it != stlmap.end());
			stlmap.erase(it);
		}
	}

	ts = gettime() - ts;
	total += ts;
	printf("delete time: %dms\n", (int)ts);

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
	benchmark("std::map", 2, COUNT);
	benchmark("linux rbtree", 1, COUNT2);
	benchmark("avlmini", 0, COUNT2);
	benchmark("std::map", 2, COUNT2);
}

void test3()
{
	benchmark("std::map", 2, 1000);
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
linux rbtree with 10000000 nodes:
insert time: 2745ms, height=33
search time: 1547ms error=0
delete time: 500ms
total: 4792ms

avlmini with 10000000 nodes:
insert time: 2852ms, height=27
search time: 1266ms error=0
delete time: 547ms
total: 4665ms

std::map with 10000000 nodes:
insert time: 3008ms
search time: 2241ms error=0
delete time: 578ms
total: 5827ms

linux rbtree with 1000000 nodes:
insert time: 234ms, height=26
search time: 110ms error=0
delete time: 31ms
total: 375ms

avlmini with 1000000 nodes:
insert time: 266ms, height=23
search time: 109ms error=0
delete time: 45ms
total: 420ms

std::map with 1000000 nodes:
insert time: 265ms
search time: 203ms error=0
delete time: 47ms
total: 515ms*/


