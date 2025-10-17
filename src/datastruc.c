#include <string.h>

#include "datastruc.h"
#include "memory.h"

struct DS_Llist *ds_llist_create(const size_t size) {
  struct DS_Llist *llist = NULL;
  if (size == 0) {
    return NULL;
  }

  llist = jalloc(sizeof(struct DS_Llist));
  if (!llist) {
    return NULL;
  }

  llist->first = NULL;
  llist->last = NULL;
  llist->it_size = size;
  llist->count = 0;

  return llist;
}

void *ds_llist_add(struct DS_Llist *llist, void **data_ptr) {
  struct DS_Llist_Node *node = NULL;
  if (!llist || !data_ptr || !*data_ptr) {
    return NULL;
  }

  node = jalloc(sizeof(struct DS_Llist_Node));
  if (!node) {
    return NULL;
  }
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
}

struct DS_Llist_Node *ds_llist_get_node(const struct DS_Llist *llist,
                                        const size_t idx) {
  size_t i = 0;
  struct DS_Llist_Node *current = NULL;
  if (!llist || llist->count <= idx) {
    return NULL;
  }

  current = llist->first;
  for (i = 0; i < idx; i++) {
    current = current->next;
  }

  return current;
}

void *ds_llist_get_data(const struct DS_Llist *llist, const size_t idx) {
  struct DS_Llist_Node *res = NULL;
  if (!llist || llist->count <= idx) {
    return NULL;
  }
  res = ds_llist_get_node(llist, idx);
  if (!res) {
    return NULL;
  } else {
    return res->data;
  }
}

void ds_llist_remove(struct DS_Llist *llist, const size_t idx,
                     ds_llist_free_node_data fn) {
  struct DS_Llist_Node *to_remove = NULL, *replacing = NULL;
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
    replacing = ds_llist_get_node(llist, idx - 1); // get 1 before
    replacing->next = NULL;
    llist->last = replacing;
  } else { // any middle
    replacing = ds_llist_get_node(llist, idx - 1);
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

void ds_llist_foreach(struct DS_Llist *llist, void (*fn)(void *)) {
  struct DS_Llist_Node *current = NULL;
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

void ds_llist_free(struct DS_Llist *llist, ds_llist_free_node_data fn) {
  if (!llist) {
    return;
  }

  while (llist->count > 0) { // clear all list nodes
    ds_llist_remove(llist, 0, fn);
  }

  jree(llist); // clear list structure
  return;
}
