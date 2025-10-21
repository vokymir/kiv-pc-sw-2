#ifndef LLIST_H
#define LLIST_H
// LINKED LIST

#include <stddef.h>

// Item of Llist
struct Llist_Node;

// Stores size of each items data.
// IS ZERO INDEXED
struct Llist;

// Function which frees data in node. Used only if the data is more complicated
// than POD, something using pointers, which cannot be freed by simply calling
// free(x).
typedef void (*llist_free_node_data)(void *);

// Create new linked list of items of size (use sizeof()).
// Size = size of one item stored in the list.
// Return reference to the Llist or NULL.
struct Llist *llist_create(const size_t size);

// Free the Llist and all its nodes. If node->data is more complex, pass some
// fn, which frees it, otherwise the jree(node->data) will be called.
void llist_free(struct Llist *llist, llist_free_node_data fn);

// Return count of nodes in given linked list, or 0 on failure.
size_t llist_count(const struct Llist *llist);

// Add new item to the Llist. List gains ownership of the item and rewoke it
// from the caller. Return pointer to newly created nodes data, or NULL.
// If adding fails, the data pointer is not cleared, caller retains ownership.
void *llist_add(struct Llist *llist, void **data_ptr);

// Get node's data at the index from llist.
// Return pointer to data, or NULL.
void *llist_get(const struct Llist *llist, const size_t idx);

// Remove node at the index from llist.
// If node->value is more complex and need specialized freeing, supplement
// function which does just that. If passed NULL, it simply call free on that
// pointer.
void llist_remove(struct Llist *llist, const size_t idx,
                  llist_free_node_data fn);

// Iterate over every node from first to last. On each node->data performs
// function fn. This function takes pointer to node->data and do stuff with it.
void llist_foreach(struct Llist *llist, void (*fn)(void *));

#endif
