#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

#include <stddef.h>

// Item of Llist
struct DS_ItLlist {
  void *data;
  struct DS_ItLlist *next;
};

// Stores size of each items data.
// IS ZERO INDEXED
struct DS_Llist {
  struct DS_ItLlist *first;
  struct DS_ItLlist *last;
  size_t it_size; // size of data in each item
  size_t count;
};

// Function pointer for freeing insides of any items data.
// DON'T free data itself, just any inside pointers. The freeing of data will be
// handled inside functions from this ds module. Takes pointer to data, return
// void.
typedef void (*ds_llist_free_it_func)(void *);

// Create new linked list of items of size (use sizeof()).
// Return reference to the Llist or NULL.
struct DS_Llist *ds_llist_new(const size_t size);

// Add new item to the Llist. List only stores reference to the item, don't
// copy it. Return pointer to newly created ItLlist->data, or NULL;
void *ds_llist_add(struct DS_Llist *llist, void *item);

// Get item at the index from llist.
// Return pointer to ItList, or NULL.
struct DS_ItLlist *ds_llist_get_it(const struct DS_Llist *llist,
                                   const size_t idx);

// Get item at the index from llist.
// Return pointer to data, or NULL.
void *ds_llist_get_data(const struct DS_Llist *llist, const size_t idx);

// Remove item at the index from llist.
// fn should remove all insides of data, but NOT data itself. It will be freed
// inside this function.
void ds_llist_remove(struct DS_Llist *llist, const size_t idx,
                     ds_llist_free_it_func fn);

// Iterate over every list item from first to last. On each items data perform
// function fn. This function takes pointer to data and do stuff with it.
void ds_llist_foreach(struct DS_Llist *llist, void (*fn)(void *));

// Free all items in list using fn, then free all memory used for this items.
// fn should remove all insides of data, but NOT data itself. It will be freed
// inside this function.
void ds_llist_free(struct DS_Llist *llist, ds_llist_free_it_func fn);

#endif
