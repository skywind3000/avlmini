#include "avlmini.c"
#include "test/linux_rbtree.c"
#include "test_avl.h"



//---------------------------------------------------------------------
// random 
//---------------------------------------------------------------------
typedef struct {
	int insert_time;
	int delete_time;
	int search_time;
	int total_time;
}	MyResult;

static void _benchmark(const char *text, int mode, int count, MyResult *r)
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
	r->insert_time = (int)ts;
	printf("insert time: %dms", (int)ts);

	if (mode == 0) {
		printf(", height=%d\n", avl_tree_height(avl_root.node));
		avl_test_validate(&avl_root);
	}
	else {
		printf(", height=%d\n", rb_tree_height(rb_root.rb_node));
	}

	sleepms(200);
	ts = gettime();

	if (mode == 0) {
		for (i = 0; i < count; i++) {
			int key = keys[count - 1 - i];
			struct MyNode dummy, *cvt;
			struct avl_node *node;
			int index;
			dummy.key = key;
			avl_node_find(&avl_root, &dummy.node, avl_node_compare, node, index);
			assert(node);
			cvt = AVL_ENTRY(node, struct MyNode, node);
			assert(cvt->key == key);
		}
	}
	else if (mode == 1) {
		for (i = 0; i < count; i++) {
			int key = keys[count - 1 - i];
			struct RbNode dummy, *cvt;
			struct rb_node *node;
			int index;
			dummy.key = key;
			rb_node_find(&rb_root, &dummy.node, rb_node_compare, node, index);
			assert(node);
			cvt = rb_entry(node, struct RbNode, node);
			assert(cvt->key == key);
		}
	}

	ts = gettime() - ts;
	total += ts;
	r->search_time = (int)ts;
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
	r->delete_time = (int)ts;
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

void result_update(MyResult *res, MyResult *newres)
{
	if (res->search_time > newres->search_time)
		res->search_time = newres->search_time;
	if (res->insert_time > newres->insert_time)
		res->insert_time = newres->insert_time;
	if (res->delete_time > newres->delete_time)
		res->delete_time = newres->delete_time;
	res->total_time = res->search_time + res->insert_time + res->delete_time;
}

void result_print(const char *name, MyResult *res)
{
	printf("%s search/insert/delete=%d/%d/%d total=%d\n",
			name, res->search_time, res->insert_time, 
			res->delete_time, res->total_time);
}

void benchmark(int count)
{
	MyResult r1, r2, t;
	int i;
	for (i = 0; i < 3; i++) {
		if (i == 0) {
			_benchmark("avlmini", 0, count, &r1);
			_benchmark("rbtree", 1, count, &r2);
		}	else {
			_benchmark("avlmini", 0, count, &t);
			result_update(&r1, &t);
			_benchmark("rbtree", 1, count, &t);
			result_update(&r2, &t);
		}
	}
	printf("count=%d:\n", count);
	result_print("avl", &r1);
	result_print("rbtree", &r2);
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
#if 1
	MyResult r;
	_benchmark("linux rbtree", 1, COUNT, &r);
	_benchmark("avlmini", 0, COUNT, &r);
	_benchmark("linux rbtree", 1, COUNT2, &r);
	_benchmark("avlmini", 0, COUNT2, &r);
	_benchmark("linux rbtree", 1, COUNT3, &r);
	_benchmark("avlmini", 0, COUNT3, &r);
#else
	benchmark(COUNT);
	benchmark(COUNT2);
	benchmark(COUNT3);
#endif
}

void test3()
{
}

int main(void)
{
#ifdef _WIN32
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
#endif
	test2();
	return 0;
}


/*
linux rbtree with 10000000 nodes:
insert time: 2124ms, height=33
search time: 1875ms
delete time: 453ms
total: 4452ms

avlmini with 10000000 nodes:
insert time: 2344ms, height=27
search time: 1422ms
delete time: 531ms
total: 4297ms

linux rbtree with 1000000 nodes:
insert time: 172ms, height=27
search time: 188ms
delete time: 31ms
total: 391ms

avlmini with 1000000 nodes:
insert time: 218ms, height=24
search time: 125ms
delete time: 47ms
total: 390ms

linux rbtree with 100000 nodes:
insert time: 16ms, height=20
search time: 15ms
delete time: 0ms
total: 31ms

avlmini with 100000 nodes:
insert time: 15ms, height=20
search time: 0ms
delete time: 0ms
total: 15ms
*/


