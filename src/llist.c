#include <string.h>

#include "common.h"
#include "llist.h"
#include "memory.h"

// ===== STRUCT DEFINITIONS =====

struct Llist_Node {
  void *data;
  struct Llist_Node *next;
};

struct Llist {
  struct Llist_Node *first;
  struct Llist_Node *last;
  size_t it_size; // size of data in each item
  size_t count;
};

// ===== PRIVATE FUNCTION DECLARATIONS =====

// Get node at the index from llist.
// Return pointer to ItList, or NULL.
static struct Llist_Node *_llist_get_node(const struct Llist *llist,
                                          const size_t idx);

// ===== PUBLIC FUNCTIONS =====

struct Llist *llist_create(const size_t size) {
  struct Llist *llist = NULL;
  if (size == 0) {
    return NULL;
  }

  llist = jalloc(sizeof(*llist));
  CLEANUP_IF_FAIL(llist);

  llist->first = NULL;
  llist->last = NULL;
  llist->it_size = size;
  llist->count = 0;

  return llist;

cleanup:
  return NULL;
}

void *llist_add(struct Llist *llist, void **data_ptr) {
  struct Llist_Node *node = NULL;
  if (!llist || !data_ptr || !*data_ptr) {
    return NULL;
  }

  node = jalloc(sizeof(*node));
  CLEANUP_IF_FAIL(node);
  node->data = *data_ptr;
  *data_ptr = NULL;
  node->next = NULL;

  if (llist->last) {
    llist->last->next = node;
  } else { // if last is NULL, count is 0
    llist->first = node;
  }

  llist->last = node;
  llist->count++;
  return node->data;

cleanup:
  return NULL;
}

void *llist_get(const struct Llist *llist, const size_t idx) {
  struct Llist_Node *res = NULL;
  if (!llist || llist->count <= idx) {
    return NULL;
  }
  res = _llist_get_node(llist, idx);
  if (!res) {
    return NULL;
  } else {
    return res->data;
  }
}

void llist_remove(struct Llist *llist, const size_t idx,
                  llist_free_node_data fn) {
  struct Llist_Node *to_remove = NULL, *replacing = NULL;
  if (!llist || llist->count == 0 || llist->count <= idx) {
    return;
  }

  if (llist->count == 1) { // edge case
    to_remove = llist->first;
    llist->first = NULL;
    llist->last = NULL;
  } else if (idx == 0) { // first item in linked list
    to_remove = llist->first;
    replacing = llist->first->next;
    llist->first = replacing;
  } else if (idx == llist->count - 1) { // last item
    to_remove = llist->last;
    replacing = _llist_get_node(llist, idx - 1); // get 1 before
    replacing->next = NULL;
    llist->last = replacing;
  } else { // any middle
    replacing = _llist_get_node(llist, idx - 1);
    to_remove = replacing->next;
    replacing->next = to_remove->next; // skip to_remove
  }
  llist->count--;

  if (fn) { // If data more complex, use fn.
    fn(to_remove->data);
  } else {
    jree(to_remove->data);
  }
  jree(to_remove);

  return;
}

void llist_foreach(struct Llist *llist, void (*fn)(void *)) {
  struct Llist_Node *current = NULL;
  if (!llist || llist->count == 0 || !fn) {
    return;
  }
  current = llist->first;

  while (current != NULL) {
    fn(current->data);
    current = current->next;
  }

  return;
}

void llist_free(struct Llist *llist, llist_free_node_data fn) {
  if (!llist) {
    return;
  }

  while (llist->count > 0) { // clear all list nodes
    llist_remove(llist, 0, fn);
  }

  jree(llist); // clear list structure
  return;
}

// ===== PRIVATE FUNCTIONS =====

static struct Llist_Node *_llist_get_node(const struct Llist *llist,
                                          const size_t idx) {
  size_t i = 0;
  struct Llist_Node *current = NULL;
  if (!llist || llist->count <= idx) {
    return NULL;
  }

  current = llist->first;
  for (i = 0; i < idx; i++) {
    current = current->next;
  }

  return current;
}
