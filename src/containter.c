#include "common.h"
#include "container.h"
#include "llist.h"
#include "memory.h"

struct Container *ct_create(const enum Container_Type type,
                            const size_t item_size) {
  struct Container *c = NULL;
  CLEANUP_IF_FAIL(type != CT_NONE && item_size != 0);

  c = jalloc(sizeof(struct Container));
  CLEANUP_IF_FAIL(c);

  switch (type) {
  case CT_LLIST:
    c->llist = llist_create(item_size);
    CLEANUP_IF_FAIL(c->llist);
    break;
  default:
    goto cleanup;
  }

  return c;

cleanup:
  if (c) {
    jree(c);
  }
  return NULL;
}

struct Container *ct_from_llist(struct Llist **l) {
  struct Container *c = NULL;
  CLEANUP_IF_FAIL(l && *l);

  c = jalloc(sizeof(struct Container));
  CLEANUP_IF_FAIL(c);

  c->type = CT_LLIST;
  c->llist = *l;
  *l = NULL;

  return c;

cleanup:
  return NULL;
}

void ct_free(struct Container *c, ct_free_item fn) {
  CLEANUP_IF_FAIL(c && c->type != CT_NONE);

  switch (c->type) {
  case CT_LLIST:
    llist_free(c->llist, fn);
    break;
  default:
    goto cleanup;
  }

cleanup:
  if (c) {
    jree(c);
  }
  return;
}

size_t ct_count(const struct Container *c) {
  CLEANUP_IF_FAIL(c && c->type != CT_NONE);

  switch (c->type) {
  case CT_LLIST:
    llist_count(c->llist);
    break;
  default:
    goto cleanup;
  }

cleanup:
  return 0;
}

// TODO: Does this work correctly, and as is described in .h file?
int ct_add(struct Container *c, void *item) {
  void *tmp = NULL;
  CLEANUP_IF_FAIL(c && c->type != CT_NONE && item);

  switch (c->type) {
  case CT_LLIST:
    tmp = llist_add(c->llist, &item);
    CLEANUP_IF_FAIL(tmp);
    return 1;
  default:
    goto cleanup;
  }

cleanup:
  return 0;
}

void *ct_get(const struct Container *c, const size_t idx) {
  CLEANUP_IF_FAIL(c && c->type != CT_NONE);

  switch (c->type) {
  case CT_LLIST:
    return llist_get(c->llist, idx);
  default:
    goto cleanup;
  }

cleanup:
  return NULL;
}

void ct_remove(struct Container *c, const size_t idx, ct_free_item fn) {
  CLEANUP_IF_FAIL(c && c->type != CT_NONE);

  switch (c->type) {
  case CT_LLIST:
    llist_remove(c->llist, idx, fn);
    break;
  default:
    goto cleanup;
  }

cleanup:
  return;
}
