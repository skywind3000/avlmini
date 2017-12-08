//=====================================================================
//
// avlhash.c - static avl-hash
//
// Created by skywind on 2017/12/08
// Last change: 2017/12/08 15:59:20
//
//=====================================================================
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "avlhash.h"


//---------------------------------------------------------------------
// initialize the hash table
//---------------------------------------------------------------------
void avl_hash_init(struct avl_hash_table *ht, 
		size_t (*hash)(const void *key),
		int (*compare)(const void *key1, const void *key2))
{
	size_t i;
	ht->count = 0;
	ht->index_size = avl_hash_INIT_SIZE;
	ht->index_mask = ht->index_size - 1;
	ht->hash = hash;
	ht->compare = compare;
	ilist_init(&ht->head);
	ht->index = ht->init;
	for (i = 0; i < avl_hash_INIT_SIZE; i++) {
		ht->index[i].avlroot.node = NULL;
		ilist_init(&(ht->index[i].node));
	}
}


//---------------------------------------------------------------------
// node traverse
//---------------------------------------------------------------------
struct avl_hash_node* avl_hash_node_first(struct avl_hash_table *ht)
{
	struct ILISTHEAD *head = ht->head.next;
	if (head != &ht->head) {
		struct avl_hash_index *index = 
			ilist_entry(head, struct avl_hash_index, node);
		struct avl_node *avlnode = avl_node_first(&index->avlroot);
		if (avlnode == NULL) return NULL;
		return AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
	}
	return NULL;
}

struct avl_hash_node* avl_hash_node_last(struct avl_hash_table *ht)
{
	struct ILISTHEAD *head = ht->head.prev;
	if (head != &ht->head) {
		struct avl_hash_index *index = 
			ilist_entry(head, struct avl_hash_index, node);
		struct avl_node *avlnode = avl_node_last(&index->avlroot);
		if (avlnode == NULL) return NULL;
		return AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
	}
	return NULL;
}

struct avl_hash_node* avl_hash_node_next(struct avl_hash_table *ht, 
		struct avl_hash_node *node)
{
	struct avl_node *avlnode;
	struct avl_hash_index *index;
	struct ILISTHEAD *listnode;
	if (node == NULL) return NULL;
	avlnode = avl_node_next(&node->avlnode);
	if (avlnode) {
		return AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
	}
	index = &(ht->index[node->hash & ht->index_mask]);
	listnode = index->node.next;
	if (listnode == &(ht->head)) {
		return NULL;
	}
	index = ilist_entry(listnode, struct avl_hash_index, node);
	avlnode = avl_node_first(&index->avlroot);
	if (avlnode == NULL) return NULL;
	return AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
}

struct avl_hash_node* avl_hash_node_prev(struct avl_hash_table *ht, 
		struct avl_hash_node *node)
{
	struct avl_node *avlnode;
	struct avl_hash_index *index;
	struct ILISTHEAD *listnode;
	if (node == NULL) return NULL;
	avlnode = avl_node_prev(&node->avlnode);
	if (avlnode) {
		return AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
	}
	index = &(ht->index[node->hash & ht->index_mask]);
	listnode = index->node.prev;
	if (listnode == &(ht->head)) {
		return NULL;
	}
	index = ilist_entry(listnode, struct avl_hash_index, node);
	avlnode = avl_node_last(&index->avlroot);
	if (avlnode == NULL) return NULL;
	return AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
}


//---------------------------------------------------------------------
// find / add / erase / replace
//---------------------------------------------------------------------
struct avl_hash_node* avl_hash_find(struct avl_hash_table *ht,
		const struct avl_hash_node *node)
{
	size_t hash = node->hash;
	const void *key = node->key;
	struct avl_hash_index *index = &(ht->index[hash & ht->index_mask]);
	struct avl_node *avlnode = index->avlroot.node;
	int (*compare)(const void *, const void *) = ht->compare;
	while (avlnode) {
		struct avl_hash_node *snode = 
			AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
		size_t shash = snode->hash;
		if (hash == shash) {
			int hc = compare(key, snode->key);
			if (hc == 0) return snode;
			avlnode = (hc < 0)? avlnode->left : avlnode->right;
		}
		else {
			avlnode = (hash < shash)? avlnode->left : avlnode->right;
		}
	}
	return NULL;
}


void avl_hash_erase(struct avl_hash_table *ht, struct avl_hash_node *node)
{
	struct avl_hash_index *index;
	ASSERTION(node && ht);
	ASSERTION(!avl_node_empty(&node->avlnode));
	index = &ht->index[node->hash & ht->index_mask];
	if (index->avlroot.node == &node->avlnode && node->avlnode.height == 1) {
		index->avlroot.node = NULL;
		ilist_del_init(&index->node);
	}
	else {
		avl_node_erase(&node->avlnode, &index->avlroot);
	}
	avl_node_init(&node->avlnode);
	ht->count--;
}

struct avl_node** avl_hash_track(struct avl_hash_table *ht,
		const struct avl_hash_node *node, struct avl_node **parent)
{
	size_t hash = node->hash;
	const void *key = node->key;
	struct avl_hash_index *index = &(ht->index[hash & ht->index_mask]);
	struct avl_node **link = &index->avlroot.node;
	struct avl_node *p = NULL;
	int (*compare)(const void *key1, const void *key2) = ht->compare;
	parent[0] = NULL;
	while (link[0]) {
		struct avl_hash_node *snode;
		size_t shash;
		p = link[0];
		snode = AVL_ENTRY(p, struct avl_hash_node, avlnode);
		shash = snode->hash;
		if (hash == shash) {
			int hc = compare(key, snode->key);
			if (hc == 0) {
				parent[0] = p;
				return NULL;
			}
			link = (hc < 0)? (&p->left) : (&p->right);
		}
		else {
			link = (hash < shash)? (&p->left) : (&p->right);
		}
	}
	parent[0] = p;
	return link;
}


struct avl_hash_node* avl_hash_add(struct avl_hash_table *ht,
		struct avl_hash_node *node)
{
	struct avl_hash_index *index = &(ht->index[node->hash & ht->index_mask]);
	if (index->avlroot.node == NULL) {
		index->avlroot.node = &node->avlnode;
		node->avlnode.parent = NULL;
		node->avlnode.left = NULL;
		node->avlnode.right = NULL;
		node->avlnode.height = 1;
		ilist_add_tail(&index->node, &ht->head);
	}
	else {
		struct avl_node **link, *parent;
		link = avl_hash_track(ht, node, &parent);
		if (link == NULL) {
			ASSERTION(parent);
			return AVL_ENTRY(parent, struct avl_hash_node, avlnode);
		}
		avl_node_link(&node->avlnode, parent, link);
		avl_node_post_insert(&node->avlnode, &index->avlroot);
	}
	ht->count++;
	return NULL;
}


void avl_hash_replace(struct avl_hash_table *ht, 
		struct avl_hash_node *victim, struct avl_hash_node *newnode)
{
	struct avl_hash_index *index = &ht->index[victim->hash & ht->index_mask];
	avl_node_replace(&victim->avlnode, &newnode->avlnode, &index->avlroot);
}

void avl_hash_clear(struct avl_hash_table *ht,
		void (*destroy)(struct avl_hash_node *node))
{
	while (!ilist_is_empty(&ht->head)) {
		struct avl_hash_index *index = ilist_entry(ht->head.next, 
				struct avl_hash_index, node);
		struct avl_node *next = NULL;
		while (index->avlroot.node != NULL) {
			struct avl_node *avlnode = avl_node_tear(&index->avlroot, &next);
			ASSERTION(avlnode);
			if (destroy) {
				struct avl_hash_node *node = 
					AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
				destroy(node);
			}
		}
		ilist_del_init(&index->node);
	}
	ht->count = 0;
}


//---------------------------------------------------------------------
// swap index memory: used for rehash
// re-index nbytes must be: sizeof(struct avl_hash_index) * n 
// n must be the power of 2
//---------------------------------------------------------------------
void* avl_hash_swap(struct avl_hash_table *ht, void *ptr, size_t nbytes)
{
	struct avl_hash_index *old_index = ht->index;
	struct avl_hash_index *new_index = (struct avl_hash_index*)ptr;
	size_t index_size = 1;
	struct ILISTHEAD head;
	size_t i;
	ASSERTION(nbytes >= sizeof(struct avl_hash_index));
	if (new_index == NULL) {
		if (ht->index == ht->init) {
			return NULL;
		}
		new_index = ht->init;
		index_size = avl_hash_INIT_SIZE;
	}
	else if (new_index == old_index) {
		return old_index;
	}
	if (new_index != ht->init) {
		size_t test_size = sizeof(struct avl_hash_index);
		while (test_size < nbytes) {
			size_t next_size = test_size * 2;
			if (next_size > nbytes) break;
			test_size = next_size;
			index_size = index_size * 2;
		}
	}
	ht->index = new_index;
	ht->index_size = index_size;
	ht->index_mask = index_size - 1;
	ht->count = 0;
	for (i = 0; i < index_size; i++) {
		ht->index[i].avlroot.node = NULL;
		ilist_init(&ht->index[i].node);
	}
	ilist_replace(&ht->head, &head);
	ilist_init(&ht->head);
	while (!ilist_is_empty(&head)) {
		struct avl_hash_index *index = ilist_entry(head.next, 
				struct avl_hash_index, node);
	#if 1
		struct avl_node *next = NULL;
		while (index->avlroot.node) {
			struct avl_node *avlnode = avl_node_tear(&index->avlroot, &next);
			struct avl_hash_node *snode, *hr;
			ASSERTION(avlnode);
			snode = AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
			hr = avl_hash_add(ht, snode);
			ASSERTION(hr == NULL);
			hr = hr;
		}
	#else
		while (index->avlroot.node) {
			struct avl_node *avlnode = index->avlroot.node;
			struct avl_hash_node *snode, *hr;
			avl_node_erase(avlnode, &index->avlroot);
			snode = AVL_ENTRY(avlnode, struct avl_hash_node, avlnode);
			hr = avl_hash_add(ht, snode);
			ASSERTION(hr == NULL);
			hr = hr;
		}
	#endif
		ilist_del_init(&index->node);
	}
	return (old_index == ht->init)? NULL : old_index;
}


//---------------------------------------------------------------------
// fastbin - fixed size object allocator
//---------------------------------------------------------------------
void avl_fastbin_init(struct avl_fastbin *fb, size_t obj_size)
{
	const size_t align = sizeof(void*);
	size_t need;
	fb->start = NULL;
	fb->endup = NULL;
	fb->next = NULL;
	fb->pages = NULL;
	fb->obj_size = (obj_size + align - 1) & (~(align - 1));
	need = fb->obj_size * 32 + sizeof(void*) + 16;	
	fb->page_size = (align <= 2)? 8 : 32;
	while (fb->page_size < need) {
		fb->page_size *= 2;
	}
	fb->maximum = (align <= 2)? fb->page_size : 0x10000;
}

void avl_fastbin_destroy(struct avl_fastbin *fb)
{
	while (fb->pages) {
		void *page = fb->pages;
		void *next = AVL_NEXT(page);
		fb->pages = next;
		free(page);
	}
	fb->start = NULL;
	fb->endup = NULL;
	fb->next = NULL;
	fb->pages = NULL;
}

void* avl_fastbin_new(struct avl_fastbin *fb)
{
	size_t obj_size = fb->obj_size;
	void *obj;
	obj = fb->next;
	if (obj) {
		fb->next = AVL_NEXT(fb->next);
		return obj;
	}
	if (fb->start + obj_size > fb->endup) {
		char *page = (char*)malloc(fb->page_size);
		size_t lineptr = (size_t)page;
		ASSERTION(page);
		AVL_NEXT(page) = fb->pages;
		fb->pages = page;
		lineptr = (lineptr + sizeof(void*) + 15) & (~15);
		fb->start = (char*)lineptr;
		fb->endup = (char*)page + fb->page_size;
		if (fb->page_size < fb->maximum) {
			fb->page_size *= 2;
		}
	}
	obj = fb->start;
	fb->start += obj_size;
	ASSERTION(fb->start <= fb->endup);
	return obj;
}

void avl_fastbin_del(struct avl_fastbin *fb, void *ptr)
{
	AVL_NEXT(ptr) = fb->next;
	fb->next = ptr;
}



//---------------------------------------------------------------------
// hash map, wrapper of avl_hash_table to support direct key/value 
//---------------------------------------------------------------------
struct avl_hash_entry* avl_map_first(struct avl_hash_map *hm)
{
	struct avl_hash_node *node = avl_hash_node_first(&hm->ht);
	if (node == NULL) return NULL;
	return AVL_ENTRY(node, struct avl_hash_entry, node);
}


struct avl_hash_entry* avl_map_last(struct avl_hash_map *hm)
{
	struct avl_hash_node *node = avl_hash_node_last(&hm->ht);
	if (node == NULL) return NULL;
	return AVL_ENTRY(node, struct avl_hash_entry, node);
}


struct avl_hash_entry* avl_map_next(struct avl_hash_map *hm, 
		struct avl_hash_entry *n)
{
	struct avl_hash_node *node = avl_hash_node_next(&hm->ht, &n->node);
	if (node == NULL) return NULL;
	return AVL_ENTRY(node, struct avl_hash_entry, node);
}


struct avl_hash_entry* avl_map_prev(struct avl_hash_map *hm, 
		struct avl_hash_entry *n)
{
	struct avl_hash_node *node = avl_hash_node_prev(&hm->ht, &n->node);
	if (node == NULL) return NULL;
	return AVL_ENTRY(node, struct avl_hash_entry, node);
}


void avl_map_init(struct avl_hash_map *hm, size_t (*hash)(const void*),
		int (*compare)(const void *, const void *))
{
	hm->count = 0;
	hm->key_copy = NULL;
	hm->key_destroy = NULL;
	hm->value_copy = NULL;
	hm->value_destroy = NULL;
	hm->insert = 0;
	hm->fixed = 0;
	avl_hash_init(&hm->ht, hash, compare);
	avl_fastbin_init(&hm->fb, sizeof(struct avl_hash_entry));
}

void avl_map_destroy(struct avl_hash_map *hm)
{
	void *ptr;
	avl_map_clear(hm);
	ptr = avl_hash_swap(&hm->ht, NULL, 0);
	if (ptr) {
		free(ptr);
	}
	avl_fastbin_destroy(&hm->fb);
}

struct avl_hash_entry* avl_map_find(struct avl_hash_map *hm, const void *key)
{
	struct avl_hash_table *ht = &hm->ht;
	struct avl_hash_node dummy;
	struct avl_hash_node *rh;
	void *ptr = (void*)key;
	avl_hash_node_key(ht, &dummy, ptr);
	rh = avl_hash_find(ht, &dummy);
	return (rh == NULL)? NULL : AVL_ENTRY(rh, struct avl_hash_entry, node);
}


void* avl_map_lookup(struct avl_hash_map *hm, const void *key, void *defval)
{
	struct avl_hash_entry *entry = avl_map_find(hm, key);
	if (entry == NULL) return defval;
	return avl_hash_value(entry);
}

static inline struct avl_hash_entry* 
avl_hash_entry_allocate(struct avl_hash_map *hm, void *key, void *value)
{
	struct avl_hash_entry *entry;
	entry = (struct avl_hash_entry*)avl_fastbin_new(&hm->fb);
	ASSERTION(entry);
	if (hm->key_copy) entry->node.key = hm->key_copy(key);
	else entry->node.key = key;
	if (hm->value_copy) entry->value = hm->value_copy(value);
	else entry->value = value;
	return entry;
}

static inline struct avl_hash_entry*
avl_hash_update(struct avl_hash_map *hm, void *key, void *value, int update)
{
	size_t hash = hm->ht.hash(key);
	struct avl_hash_index *index = &(hm->ht.index[hash & hm->ht.index_mask]);
	struct avl_node **link = &index->avlroot.node;
	struct avl_node *parent = NULL;
	struct avl_hash_entry *entry;
	int (*compare)(const void *key1, const void *key2) = hm->ht.compare;
	if (index->avlroot.node == NULL) {
		entry = avl_hash_entry_allocate(hm, key, value);
		ASSERTION(entry);
		entry->node.avlnode.height = 1;
		entry->node.avlnode.left = NULL;
		entry->node.avlnode.right = NULL;
		entry->node.avlnode.parent = NULL;
		entry->node.hash = hash;
		index->avlroot.node = &(entry->node.avlnode);
		ilist_add_tail(&index->node, &(hm->ht.head));
		hm->ht.count++;
		hm->insert = 1;
		return entry;
	}
	while (link[0]) {
		struct avl_hash_node *snode;
		size_t shash;
		parent = link[0];
		snode = AVL_ENTRY(parent, struct avl_hash_node, avlnode);
		shash = snode->hash;
		if (hash != shash) {
			link = (hash < shash)? (&parent->left) : (&parent->right);
		}	else {
			int hc = compare(key, snode->key);
			if (hc == 0) {
				entry = AVL_ENTRY(snode, struct avl_hash_entry, node);
				if (update) {
					if (hm->value_destroy) {
						hm->value_destroy(entry->value);
					}
					if (hm->value_copy == NULL) entry->value = value;
					else entry->value = hm->value_copy(value);
				}
				hm->insert = 0;
				return entry;
			}	else {
				link = (hc < 0)? (&parent->left) : (&parent->right);
			}
		}
	}
	entry = avl_hash_entry_allocate(hm, key, value);
	ASSERTION(entry);
	entry->node.hash = hash;
	avl_node_link(&(entry->node.avlnode), parent, link);
	avl_node_post_insert(&(entry->node.avlnode), &index->avlroot);
	hm->ht.count++;
	hm->insert = 1;
	return entry;
}

static inline void avl_map_rehash(struct avl_hash_map *hm, size_t capacity)
{
	size_t isize = hm->ht.index_size;
	size_t limit = (capacity * 6) >> 2;    /* capacity * 6 / 4 */
	if (isize < limit && hm->fixed == 0) {
		size_t need = isize;
		size_t size;
		void *ptr;
		while (need < limit) need <<= 1;
		size = need * sizeof(struct avl_hash_index);
		ptr = malloc(size);
		ASSERTION(ptr);
		ptr = avl_hash_swap(&hm->ht, ptr, size);
		if (ptr) {
			free(ptr);
		}
	}
}

void avl_map_reserve(struct avl_hash_map *hm, size_t capacity)
{
	avl_map_rehash(hm, capacity);
}

struct avl_hash_entry* 
avl_map_add(struct avl_hash_map *hm, void *key, void *value, int *success)
{
	struct avl_hash_entry *entry = avl_hash_update(hm, key, value, 0);
	if (success) success[0] = hm->insert;
	avl_map_rehash(hm, hm->ht.count);
	return entry;
}

struct avl_hash_entry*
avl_map_set(struct avl_hash_map *hm, void *key, void *value)
{
	struct avl_hash_entry *entry = avl_hash_update(hm, key, value, 0);
	avl_map_rehash(hm, hm->ht.count);
	return entry;
}

void *avl_map_get(struct avl_hash_map *hm, const void *key)
{
	return avl_map_lookup(hm, key, NULL);
}

void avl_map_erase(struct avl_hash_map *hm, struct avl_hash_entry *entry)
{
	ASSERTION(entry);
	ASSERTION(!avl_node_empty(&(entry->node.avlnode)));
	avl_hash_erase(&hm->ht, &entry->node);
	avl_node_init(&(entry->node.avlnode));
	if (hm->key_destroy) hm->key_destroy(entry->node.key);
	if (hm->value_destroy) hm->value_destroy(entry->value);
	entry->node.key = NULL;
	entry->value = NULL;
	avl_fastbin_del(&hm->fb, entry);
}

int avl_map_remove(struct avl_hash_map *hm, const void *key)
{
	struct avl_hash_entry *entry;
	entry = avl_map_find(hm, key);
	if (entry == NULL) {
		return -1;
	}
	avl_map_erase(hm, entry);
	return 0;
}

void avl_map_clear(struct avl_hash_map *hm)
{
	while (1) {
		struct avl_hash_entry *entry = avl_map_first(hm);
		if (entry == NULL) break;
		avl_map_erase(hm, entry);
	}
	ASSERTION(hm->count == 0);
}


