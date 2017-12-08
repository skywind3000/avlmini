//=====================================================================
//
// avlhash_inline.c - hash / compare functions and inline searcher
//
// Created by skywind on 2017/12/08
// Last change: 2017/12/08 16:10:32
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avlhash_funcs.h"


//---------------------------------------------------------------------
// hash functions for types
//---------------------------------------------------------------------
size_t avl_hash_seed = 0x11223344;

size_t avl_hash_func_uint(const void *key)
{
#if 0
	size_t x = (size_t)key;
	return (x * 2654435761u) ^ avl_hash_seed;
#else
	return (size_t)key;
#endif
}

size_t avl_hash_func_int(const void *key)
{
#if 0
	size_t x = (size_t)key;
	return (x * 2654435761u) ^ avl_hash_seed;
#else
	return (size_t)key;
#endif
}

size_t avl_hash_bytes_stl(const void *ptr, size_t size, size_t seed)
{
	const unsigned char *buf = (const unsigned char*)ptr;
	const size_t m = 0x5bd1e995;
	size_t hash = size ^ seed;
	for (; size >= 4; buf += 4, size -= 4) {
		size_t k = *((unsigned int*)buf);
		k *= m;
		k = (k >> 24) * m;
		hash = (hash * m) ^ k;
	}
	switch (size) {
	case 3: hash ^= ((unsigned int)buf[2]) << 16;
	case 2: hash ^= ((unsigned int)buf[1]) << 8;
	case 1: hash ^= ((unsigned int)buf[0]); hash = hash * m; break;
	}
	hash = (hash ^ (hash >> 13)) * m;
	return hash ^ (hash >> 15);
}

size_t avl_hash_bytes_lua(const void *ptr, size_t size, size_t seed)
{
	const unsigned char *name = (const unsigned char*)ptr;
	size_t step = (size >> 5) + 1;
	size_t h = size ^ seed, i;
    for(i = size; i >= step; i -= step)
        h = h ^ ((h << 5) + (h >> 2) + (size_t)name[i - 1]);
    return h;
}

size_t avl_hash_func_cstr(const void *key)
{
	const char *str = (const char*)key;
	size_t size = strlen(str);
#ifndef HASH_BYTES_STL
	return avl_hash_bytes_lua(str, size, avl_hash_seed);
#else
	return avl_hash_bytes_stl(str, size, avl_hash_seed);
#endif
}


//---------------------------------------------------------------------
// compare functions for types
//---------------------------------------------------------------------
int avl_hash_compare_uint(const void *key1, const void *key2)
{
	size_t x = (size_t)key1;
	size_t y = (size_t)key2;
	if (x == y) return 0;
	return (x < y)? -1 : 1;
}

int avl_hash_compare_int(const void *key1, const void *key2)
{
	ptrdiff_t x = (ptrdiff_t)key1;
	ptrdiff_t y = (ptrdiff_t)key2;
	if (x == y) return 0;
	return (x < y)? -1 : 1;
}

int avl_compare_bytes(const void *p1, size_t s1, const void *p2, size_t s2)
{
	int minsize = (s1 < s2)? s1 : s2;
	int hr = memcmp(p1, p2, minsize);
	if (hr == 0) {
		if (s1 == s2) return 0;
		return (s1 < s2)? -1 : 1;
	}
	else {
		return (hr < 0)? -1 : 1;
	}
}

int avl_hash_compare_cstr(const void *key1, const void *key2)
{
	const char *x = (const char*)key1;
	const char *y = (const char*)key2;
	return avl_compare_bytes(x, strlen(x), y, strlen(y));
}


//---------------------------------------------------------------------
// uses ib_map_search template for inline searching, faster than 
// directly invoking avl_map_find
//---------------------------------------------------------------------
struct avl_hash_entry *avl_map_find_uint(struct avl_hash_map *hm, unsigned int key)
{
	struct avl_hash_entry *hr;
	void *kk = (void*)key;
	avl_map_search(hm, kk, avl_hash_func_uint, avl_hash_compare_uint, hr);
	return hr;
}

struct avl_hash_entry *avl_map_find_int(struct avl_hash_map *hm, int key)
{
	struct avl_hash_entry *hr;
	void *kk = (void*)key;
	avl_map_search(hm, kk, avl_hash_func_int, avl_hash_compare_int, hr);
	return hr;
}

struct avl_hash_entry *avl_map_find_cstr(struct avl_hash_map *hm, const char *key)
{
	struct avl_hash_entry *hr;
	void *kk = (void*)key;
	avl_map_search(hm, kk, avl_hash_func_cstr, avl_hash_compare_cstr, hr);
	return hr;
}




