#ifndef LLIST_H
#define LLIST_H

#include <stddef.h>

// Item of Llist
struct Llist_Node {
  void *data;
  struct Llist_Node *next;
};

// Stores size of each items data.
// IS ZERO INDEXED
struct Llist {
  struct Llist_Node *first;
  struct Llist_Node *last;
  size_t it_size; // size of data in each item
  size_t count;
};

// Function which frees data in node. Used only if the data is more complicated
// than POD, something using pointers, which cannot be freed by simply calling
// free(x).
typedef void (*llist_free_node_data)(void *);

// Create new linked list of items of size (use sizeof()).
// Return reference to the Llist or NULL.
struct Llist *llist_create(const size_t size);

// Add new item to the Llist. List gains ownership of the item and rewoke it
// from the caller. Return pointer to newly created ItLlist->data, or NULL.
// If adding fails, the item pointer is not cleared.
void *llist_add(struct Llist *llist, void **data_ptr);

// Get node at the index from llist.
// Return pointer to ItList, or NULL.
struct Llist_Node *llist_get_node(const struct Llist *llist, const size_t idx);

// Get node's data at the index from llist.
// Return pointer to data, or NULL.
void *llist_get_data(const struct Llist *llist, const size_t idx);

// Remove node at the index from llist.
// If node->value is more complex and need specialized freeing, supplement
// function which does just that. If passed NULL, it simply call free on that
// pointer.
void llist_remove(struct Llist *llist, const size_t idx,
                  llist_free_node_data fn);

// Iterate over every node from first to last. On each node->data performs
// function fn. This function takes pointer to node->data and do stuff with it.
void llist_foreach(struct Llist *llist, void (*fn)(void *));

// Free the Llist and all its nodes. If node->data is more complex, pass some
// fn, which frees it, otherwise the jree(node->data) will be called.
void llist_free(struct Llist *llist, llist_free_node_data fn);

#endif
