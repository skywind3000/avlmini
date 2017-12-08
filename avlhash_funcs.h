//=====================================================================
//
// avlhash_inline.h - hash / compare functions and inline searcher
//
// Created by skywind on 2017/12/08
// Last change: 2017/12/08 16:10:32
//
//=====================================================================
#ifndef __AVLHASH_FUNCS_H__
#define __AVLHASH_FUNCS_H__

#include "avlhash.h"

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
// hash/compare functions for types
//---------------------------------------------------------------------
size_t avl_hash_func_uint(const void *key);
int avl_hash_compare_uint(const void *key1, const void *key2);

size_t avl_hash_func_int(const void *key);
int avl_hash_compare_int(const void *key1, const void *key2);

size_t avl_hash_func_cstr(const void *key);
int avl_hash_compare_cstr(const void *key1, const void *key2);


//---------------------------------------------------------------------
// uses ib_map_search template for inline searching, faster than 
// directly invoking avl_map_find
//---------------------------------------------------------------------
struct avl_hash_entry *avl_map_find_uint(struct avl_hash_map *hm, unsigned int key);
struct avl_hash_entry *avl_map_find_int(struct avl_hash_map *hm, int key);
struct avl_hash_entry *avl_map_find_cstr(struct avl_hash_map *hm, const char *key);




#ifdef __cplusplus
}
#endif

#endif



