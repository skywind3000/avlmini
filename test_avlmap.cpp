//=====================================================================
//
// test_avlmap.cpp - test avl_hash_map
//
// Created by skywind on 2017/12/08
// Last change: 2017/12/08 16:48:39
//
//=====================================================================
#include "avlhash.h"

#include "avlhash.c"
#include "avlmini.c"

#include "test_avl.h"


// open this for collision test
#if 0
	#define SAME_HASH
#endif

#ifndef SAME_HASH
struct NodeHash { size_t operator()(const int& rhs) const { return (size_t)rhs; } };
#else
struct NodeHash { size_t operator()(const int& rhs) const { return 0; } };
#endif

#ifdef _MSC_VER
#include <unordered_map>
typedef std::unordered_map<int, int, NodeHash> map_type;
#elif (__GNUC__ <= 4) && (__GNUC_MINOR__ <= 7)
#include <tr1/memory>
#include <tr1/unordered_map>
typedef std::tr1::unordered_map<int, int, NodeHash> map_type;
#else
#include <memory>
#include <unordered_map>
typedef std::unordered_map<int, int, NodeHash> map_type;
#endif


int node_compare(const void *key1, const void *key2)
{
#ifndef SAME_HASH
	size_t x = (size_t)key1;
	size_t y = (size_t)key2;
	return (int)(x - y);
#else
	return 0;
#endif
}

size_t node_hash(const void *key)
{
	return (size_t)key;
}


//---------------------------------------------------------------------
// benchmark
//---------------------------------------------------------------------
void benchmark(const char *name, int mode, int count)
{
	int *keys = new int[count * 2];
	int *search = keys + count;
	struct avl_hash_map hmap;
	map_type umap;
	unsigned int ts;

	printf("benchmark %s:\n", name);
	random_keys(keys, count, 0x11223344);
	random_keys(search, count, 0x55667788);

	avl_map_init(&hmap, node_hash, node_compare);
	/* hmap.builtin = 10; */
	/* printf("builtin=%d\n", hmap.builtin); */
	/* avl_map_reserve(&hmap, count); */

#ifdef _MSC_VER
	umap.reserve(count);
#elif __GNUC__ > 4
	umap.reserve(count);
#endif
	
	avl_map_reserve(&hmap, count);

	printf("insert time: ");
	sleepms(100);

	ts = gettime();
	if (mode == 0) {
		for (int i = 0; i < count; i++) {
			int key = keys[i];
			avl_map_set(&hmap, (void*)key, (void*)(key * 10));
		}
	}
	else if (mode == 1) {
		for (int i = 0; i < count; i++) {
			int key = keys[i];
			umap[key] = key * 10;
		}
	}
	ts = gettime() - ts;
	printf("%dms\n", (int)ts);
	/* printf("%d\n", itemnum); */

	avl_map_reserve(&hmap, count);

	printf("search time: ");
	sleepms(100);
	ts = gettime();

	if (mode == 0) {
		for (int i = 0; i < count; i++) {
			int key = search[i];
			avl_hash_entry *entry;
		#ifdef NO_INLINE_TEMPLATE
			// standard: call hash() / compare() with function pointer
			entry = avl_map_find(&hmap, ((void*)key)); 
		#else
			// template: call hash() / compare() inline
			avl_map_search(&hmap, ((void*)key), node_hash, node_compare, entry);
		#endif
			int val = (int)entry->value;
			assert(entry);
			if (val != key * 10) {
				printf("key=%d val=%d\n", key, val);
				printf("error \n");
				return;
				assert(val == key * 10);
			}
		}
	}
	else if (mode == 1) {
		for (int i = 0; i < count; i++) {
			int key = search[i];
			map_type::iterator it = umap.find(key);
			assert(it != umap.end());
			assert(it->second == key * 10);
		}
	}

	ts = gettime() - ts;
	printf("%dms\n", (int)ts);

	printf("delete time: ");
	sleepms(100);
	ts = gettime();

	if (mode == 0) {
		for (int i = 0; i < count; i++) {
			int key = search[count - 1 - i];
			int hr = avl_map_remove(&hmap, (void*)key);
			assert(hr == 0);
		}
	}
	else if (mode == 1) {
		for (int i = 0; i < count; i++) {
			int key = search[count - 1 - i];
			map_type::iterator it = umap.find(key);
			assert(it != umap.end());
			umap.erase(it);
		}
	}

	ts = gettime() - ts;
	printf("%dms\n", (int)ts);

	delete keys;
	printf("\n");
}



//---------------------------------------------------------------------
// standard test
//---------------------------------------------------------------------
void test_standard()
{
#define TTIMES 10000000
	benchmark("hash", 0, TTIMES);
	benchmark("unordered_map", 1, TTIMES);
}



//---------------------------------------------------------------------
// collision test
//---------------------------------------------------------------------
void test_collision()
{
	for (int i = 1000; i <= 30000; i += 1000) {
		printf("\n<%d>\n", i);
		benchmark("hash", 0, i);
		benchmark("unordered_map", 1, i);
	}
}


//---------------------------------------------------------------------
// program entry
//---------------------------------------------------------------------
int main(void)
{
#ifndef SAME_HASH
	test_standard();
#else
	test_collision();
#endif
	return 0;
}



