#ifndef CONTAINER_H
#define CONTAINER_H

#include <stddef.h>

#include "llist.h"
enum Container_Type {
  CT_NONE,
  CT_LLIST,
};

struct Container {
  enum Container_Type type;
  union {
    struct Llist *llist;
  } u;
};

typedef void (*ct_free_item)(void *);

// Create a new container using given data type, and item_size.
// Item_size is how big one item will be.
// Return pointer to new container, or NULL on failure.
struct Container *ct_create(const enum Container_Type type,
                            const size_t item_size);

// Create new container from existing Linked list.
// Transfers ownership from caller to Container, clear l pointer.
// Return pointer to container or NULL on failute.
// In case of failure, original l pointer is not cleared, ownership not
// transferred.
struct Container *ct_from_llist(struct Llist **l);

// Free container and all its insides. If provided, uses fn to free items.
// Useful, if items are more complex than POD. Otherwise calls jree.
void ct_free(struct Container *c, ct_free_item fn);

// Return number of items in container. On failure return 0.
size_t ct_count(const struct Container *c);

// Add new item to container.
// Set *item = NULL to transfer ownership to container. (only on success)
// Return pointer to item on success, NULL on failure.
void *ct_add(struct Container *c, void **item_ptr);

// Get item at index idx.
// Return pointer or NULL on failure.
void *ct_get(const struct Container *c, const size_t idx);

// Remove item at index. If fn is provided, uses it to free the item. Useful for
// more complex structures than POD.
void ct_remove(struct Container *c, const size_t idx, ct_free_item fn);

// Iterate over each item in container
// Calls fn(item) for each item.
void ct_foreach(const struct Container *c, void (*fn)(void *));

#endif
