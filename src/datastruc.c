#include <string.h>

#include "datastruc.h"
#include "memory.h"

struct DS_Llist *ds_llist_new(const size_t size) {
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

void *ds_llist_add(struct DS_Llist *llist, const void *item) {
  struct DS_ItLlist *lit = NULL;
  if (!llist || !item) {
    return NULL;
  }

  lit = jalloc(sizeof(struct DS_ItLlist));
  if (!lit) {
    return NULL;
  }
  lit->data = NULL;
  lit->next = NULL;
  lit->data = jalloc(llist->it_size);

  if (!lit->data) {
    jree(lit);
    return NULL;
  }

  memcpy(lit->data, item, llist->it_size);

  if (!llist->last) {
    llist->first = lit;
    llist->last = lit;
    llist->count = 1;
    return lit->data;
  }

  llist->last->next = lit;
  llist->last = lit;
  llist->count++;
  return lit->data;
}

struct DS_ItLlist *ds_llist_get_it(const struct DS_Llist *llist,
                                   const size_t idx) {
  size_t i = 0;
  struct DS_ItLlist *current = NULL;
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
  struct DS_ItLlist *res = NULL;
  if (!llist || llist->count <= idx) {
    return NULL;
  }
  res = ds_llist_get_it(llist, idx);
  if (!res) {
    return NULL;
  } else {
    return res->data;
  }
}

void ds_llist_remove(struct DS_Llist *llist, const size_t idx,
                     const ds_llist_free_it_func fn) {
  struct DS_ItLlist *to_remove, *replacing = NULL;
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
    replacing = ds_llist_get_it(llist, idx - 1); // get 1 before
    replacing->next = NULL;
    llist->last = replacing;
  } else { // any middle
    replacing = ds_llist_get_it(llist, idx - 1);
    to_remove = replacing->next;
    replacing->next = to_remove->next; // skip to_remove
  }
  llist->count--;
  if (fn) {
    fn(to_remove->data);
  }
  jree(to_remove->data);
  jree(to_remove);

  return;
}

void ds_llist_foreach(struct DS_Llist *llist, void (*fn)(void *)) {
  struct DS_ItLlist *current = NULL;
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

void ds_llist_free(struct DS_Llist *llist, const ds_llist_free_it_func fn) {
  struct DS_ItLlist *current, *next = NULL;
  if (!llist) {
    return;
  }
  current = llist->first;

  while (current != NULL) { // clear all list items
    next = current->next;
    if (fn) {
      fn(current->data);
    }
    jree(current->data);
    jree(current);
    current = next;
  }

  llist->first = NULL;
  llist->last = NULL;
  jree(llist); // clear list structure
  return;
}
