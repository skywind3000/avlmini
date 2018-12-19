//=====================================================================
//
// avlhash.h - static avl-hash
//
// Created by skywind on 2017/12/08
// Last change: 2017/12/08 15:58:40
//
//=====================================================================
#ifndef __AVLHASH_H__
#define __AVLHASH_H__

#include "avlmini.h"


/*====================================================================*/
/* LIST DEFINITION                                                   */
/*====================================================================*/
#ifndef __ILIST_DEF__
#define __ILIST_DEF__

struct ILISTHEAD {
	struct ILISTHEAD *next, *prev;
};

typedef struct ILISTHEAD ilist_head;


/*--------------------------------------------------------------------*/
/* list init                                                          */
/*--------------------------------------------------------------------*/
#define ILIST_HEAD_INIT(name) { &(name), &(name) }
#define ILIST_HEAD(name) \
	struct ILISTHEAD name = ILIST_HEAD_INIT(name)

#define ILIST_INIT(ptr) ( \
	(ptr)->next = (ptr), (ptr)->prev = (ptr))

#define IOFFSETOF(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define ICONTAINEROF(ptr, type, member) ( \
		(type*)( ((char*)((type*)ptr)) - IOFFSETOF(type, member)) )

#define ILIST_ENTRY(ptr, type, member) ICONTAINEROF(ptr, type, member)


/*--------------------------------------------------------------------*/
/* list operation                                                     */
/*--------------------------------------------------------------------*/
#define ILIST_ADD(node, head) ( \
	(node)->prev = (head), (node)->next = (head)->next, \
	(head)->next->prev = (node), (head)->next = (node))

#define ILIST_ADD_TAIL(node, head) ( \
	(node)->prev = (head)->prev, (node)->next = (head), \
	(head)->prev->next = (node), (head)->prev = (node))

#define ILIST_DEL_BETWEEN(p, n) ((n)->prev = (p), (p)->next = (n))

#define ILIST_DEL(entry) (\
	(entry)->next->prev = (entry)->prev, \
	(entry)->prev->next = (entry)->next, \
	(entry)->next = 0, (entry)->prev = 0)

#define ILIST_DEL_INIT(entry) do { \
	ILIST_DEL(entry); ILIST_INIT(entry); } while (0)

#define ILIST_IS_EMPTY(entry) ((entry) == (entry)->next)

#define ilist_init		ILIST_INIT
#define ilist_entry		ILIST_ENTRY
#define ilist_add		ILIST_ADD
#define ilist_add_tail	ILIST_ADD_TAIL
#define ilist_del		ILIST_DEL
#define ilist_del_init	ILIST_DEL_INIT
#define ilist_is_empty	ILIST_IS_EMPTY

#define ILIST_FOREACH(iterator, head, TYPE, MEMBER) \
	for ((iterator) = ilist_entry((head)->next, TYPE, MEMBER); \
		&((iterator)->MEMBER) != (head); \
		(iterator) = ilist_entry((iterator)->MEMBER.next, TYPE, MEMBER))

#define ilist_foreach(iterator, head, TYPE, MEMBER) \
	ILIST_FOREACH(iterator, head, TYPE, MEMBER)

#define ilist_foreach_entry(pos, head) \
	for( (pos) = (head)->next; (pos) != (head) ; (pos) = (pos)->next )
	

#define __ilist_splice(list, head) do {	\
		ilist_head *first = (list)->next, *last = (list)->prev; \
		ilist_head *at = (head)->next; \
		(first)->prev = (head), (head)->next = (first);		\
		(last)->next = (at), (at)->prev = (last); }	while (0)

#define ilist_splice(list, head) do { \
	if (!ilist_is_empty(list)) __ilist_splice(list, head); } while (0)

#define ilist_splice_init(list, head) do {	\
	ilist_splice(list, head);	ilist_init(list); } while (0)

#define ilist_replace(oldnode, newnode) ( \
	(newnode)->next = (oldnode)->next, \
	(newnode)->next->prev = (newnode), \
	(newnode)->prev = (oldnode)->prev, \
	(newnode)->prev->next = (newnode))

#ifdef _MSC_VER
#pragma warning(disable:4311)
#pragma warning(disable:4312)
#pragma warning(disable:4996)
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------
// avl_hash_node: embedded in your structure
//---------------------------------------------------------------------
struct avl_hash_node
{
	struct avl_node avlnode;	// avltree node
	void *key;					// generic type pointer
	size_t hash;
};


//---------------------------------------------------------------------
// hash index (or slot/bucket)
//---------------------------------------------------------------------
struct avl_hash_index
{
	struct ILISTHEAD node;		// linked to the list for non-empty indexes
	struct avl_root avlroot;	// avl root
};

#define avl_hash_INIT_SIZE    8


//---------------------------------------------------------------------
// static hash table: zero memory allocation
//---------------------------------------------------------------------
struct avl_hash_table
{
	size_t count;				// node count
	size_t index_size;			// must be the power of 2
	size_t index_mask;			// must be (index_size - 1);
	size_t (*hash)(const void *key);
	int (*compare)(const void *key1, const void *key2);
	struct ILISTHEAD head;
	struct avl_hash_index *index;
	struct avl_hash_index init[avl_hash_INIT_SIZE];
};


void avl_hash_init(struct avl_hash_table *ht, 
		size_t (*hash)(const void *key),
		int (*compare)(const void *key1, const void *key2));


//---------------------------------------------------------------------
// node traverse
//---------------------------------------------------------------------
struct avl_hash_node* avl_hash_node_first(struct avl_hash_table *ht);
struct avl_hash_node* avl_hash_node_last(struct avl_hash_table *ht);

struct avl_hash_node* avl_hash_node_next(struct avl_hash_table *ht, 
		struct avl_hash_node *node);

struct avl_hash_node* avl_hash_node_prev(struct avl_hash_table *ht, 
		struct avl_hash_node *node);


//---------------------------------------------------------------------
// setup key
//---------------------------------------------------------------------
static inline void avl_hash_node_key(struct avl_hash_table *ht, 
		struct avl_hash_node *node, void *key) {
	node->key = key;
	node->hash = ht->hash(key);
}


//---------------------------------------------------------------------
// find / add / erase / replace
//---------------------------------------------------------------------
struct avl_hash_node* avl_hash_find(struct avl_hash_table *ht,
		const struct avl_hash_node *node);

struct avl_node** avl_hash_track(struct avl_hash_table *ht,
		const struct avl_hash_node *node, struct avl_node **parent);

struct avl_hash_node* avl_hash_add(struct avl_hash_table *ht,
		struct avl_hash_node *node);

void avl_hash_erase(struct avl_hash_table *ht, struct avl_hash_node *node);

void avl_hash_replace(struct avl_hash_table *ht, 
		struct avl_hash_node *victim, struct avl_hash_node *newnode);

void avl_hash_clear(struct avl_hash_table *ht,
		void (*destroy)(struct avl_hash_node *node));



//---------------------------------------------------------------------
// swap index memory: used for rehash
// re-index nbytes must be: sizeof(struct avl_hash_index) * n 
// n must be the power of 2
//---------------------------------------------------------------------
void* avl_hash_swap(struct avl_hash_table *ht, void *index, size_t nbytes);




//---------------------------------------------------------------------
// fastbin - fixed size object allocator
//---------------------------------------------------------------------
struct avl_fastbin
{
	size_t obj_size;
	size_t page_size;
	size_t maximum;
	char *start;
	char *endup;
	void *next;
	void *pages;
};


#define AVL_NEXT(ptr)  (((void**)(ptr))[0])

void avl_fastbin_init(struct avl_fastbin *fb, size_t obj_size);
void avl_fastbin_destroy(struct avl_fastbin *fb);

void* avl_fastbin_new(struct avl_fastbin *fb);
void avl_fastbin_del(struct avl_fastbin *fb, void *ptr);


//---------------------------------------------------------------------
// hash map (wrapper of hash table)
//---------------------------------------------------------------------
struct avl_hash_entry
{
	struct avl_hash_node node;
	void *value;
};

struct avl_hash_map
{
	size_t count;
	int insert;
	int fixed;
	int builtin;
	void* (*key_copy)(void *key);
	void (*key_destroy)(void *key);
	void* (*value_copy)(void *value);
	void (*value_destroy)(void *value);
	struct avl_fastbin fb;
	struct avl_hash_table ht;
};


#define avl_hash_key(entry)     ((entry)->node.key)
#define avl_hash_value(entry)   ((entry)->value)

void avl_map_init(struct avl_hash_map *hm, size_t (*hash)(const void*),
		int (*compare)(const void *, const void *));

void avl_map_destroy(struct avl_hash_map *hm);

struct avl_hash_entry* avl_map_first(struct avl_hash_map *hm);
struct avl_hash_entry* avl_map_last(struct avl_hash_map *hm);

struct avl_hash_entry* avl_map_next(struct avl_hash_map *hm, 
		struct avl_hash_entry *n);
struct avl_hash_entry* avl_map_prev(struct avl_hash_map *hm, 
		struct avl_hash_entry *n);

struct avl_hash_entry* avl_map_find(struct avl_hash_map *hm, const void *key);
void* avl_map_lookup(struct avl_hash_map *hm, const void *key, void *defval);


struct avl_hash_entry* avl_map_add(struct avl_hash_map *hm, 
		void *key, void *value, int *success);

struct avl_hash_entry* avl_map_set(struct avl_hash_map *hm,
		void *key, void *value);

void* avl_map_get(struct avl_hash_map *hm, const void *key);

void avl_map_erase(struct avl_hash_map *hm, struct avl_hash_entry *entry);


/* returns 0 for success, -1 for key mismatch */
int avl_map_remove(struct avl_hash_map *hm, const void *key);

void avl_map_clear(struct avl_hash_map *hm);


/*--------------------------------------------------------------------*/
/* fast inline search template                                        */
/*--------------------------------------------------------------------*/
#define avl_map_search(hm, srckey, hash_func, cmp_func, result) do { \
		size_t __hash = (hash_func)(srckey); \
		struct avl_hash_index *__index = \
			&((hm)->ht.index[__hash & ((hm)->ht.index_mask)]); \
		struct avl_node *__anode = __index->avlroot.node; \
		(result) = NULL; \
		while (__anode) { \
			struct avl_hash_node *__snode = \
				AVL_ENTRY(__anode, struct avl_hash_node, avlnode); \
			size_t __shash = __snode->hash; \
			if (__hash == __shash) { \
				int __hc = (cmp_func)((srckey), __snode->key); \
				if (__hc == 0) { \
					(result) = AVL_ENTRY(__snode, \
							struct avl_hash_entry, node);\
					break; \
				} \
				__anode = (__hc < 0)? __anode->left : __anode->right; \
			}	else { \
				__anode = (__hash < __shash)? __anode->left:__anode->right;\
			} \
		} \
	}	while (0)


#ifdef __cplusplus
}
#endif


#endif



