#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stddef.h>

// Item of Llist
struct DS_Llist_Node {
  void *data;
  struct DS_Llist_Node *next;
};

// Stores size of each items data.
// IS ZERO INDEXED
struct DS_Llist {
  struct DS_Llist_Node *first;
  struct DS_Llist_Node *last;
  size_t it_size; // size of data in each item
  size_t count;
};

// Function which frees data in node. Used only if the data is more complicated
// than POD, something using pointers, which cannot be freed by simply calling
// free(x).
typedef void (*ds_llist_free_node_data)(void *);

// Create new linked list of items of size (use sizeof()).
// Return reference to the Llist or NULL.
struct DS_Llist *ds_llist_create(const size_t size);

// Add new item to the Llist. List gains ownership of the item and rewoke it
// from the caller. Return pointer to newly created ItLlist->data, or NULL.
// If adding fails, the item pointer is not cleared.
void *ds_llist_add(struct DS_Llist *llist, void **data_ptr);

// Get node at the index from llist.
// Return pointer to ItList, or NULL.
struct DS_Llist_Node *ds_llist_get_node(const struct DS_Llist *llist,
                                        const size_t idx);

// Get node's data at the index from llist.
// Return pointer to data, or NULL.
void *ds_llist_get_data(const struct DS_Llist *llist, const size_t idx);

// Remove node at the index from llist.
// If node->value is more complex and need specialized freeing, supplement
// function which does just that. If passed NULL, it simply call free on that
// pointer.
void ds_llist_remove(struct DS_Llist *llist, const size_t idx,
                     ds_llist_free_node_data fn);

// Iterate over every node from first to last. On each node->data performs
// function fn. This function takes pointer to node->data and do stuff with it.
void ds_llist_foreach(struct DS_Llist *llist, void (*fn)(void *));

// Free the Llist and all its nodes. If node->data is more complex, pass some
// fn, which frees it, otherwise the jree(node->data) will be called.
void ds_llist_free(struct DS_Llist *llist, ds_llist_free_node_data fn);

#endif
