#include "common.h"
#include "container.h"
#include "llist.h"
#include "memory.h"

struct Container *ct_create(const enum Container_Type type,
                            const size_t item_size) {
  struct Container *c = NULL;
  CLEANUP_IF_FAIL(item_size != 0);

  c = jalloc(sizeof(struct Container));
  CLEANUP_IF_FAIL(c);

  switch (type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    c->u.llist = llist_create(item_size);
    CLEANUP_IF_FAIL(c->u.llist);
    c->type = CT_LLIST;
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
  c->u.llist = *l;
  *l = NULL;

  return c;

cleanup:
  return NULL;
}

void ct_free(struct Container *c, ct_free_item fn) {
  CLEANUP_IF_FAIL(c);

  switch (c->type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    llist_free(c->u.llist, fn);
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
  CLEANUP_IF_FAIL(c);

  switch (c->type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    return llist_count(c->u.llist);
    break;
  default:
    goto cleanup;
  }

cleanup:
  return 0;
}

void *ct_add(struct Container *c, void **item_ptr) {
  void *tmp = NULL;
  CLEANUP_IF_FAIL(c && item_ptr && *item_ptr);

  switch (c->type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    tmp = llist_add(c->u.llist, item_ptr);
    break;
  default:
    goto cleanup;
  }

  CLEANUP_IF_FAIL(tmp);
  return tmp;

cleanup:
  return NULL;
}

void *ct_get(const struct Container *c, const size_t idx) {
  CLEANUP_IF_FAIL(c);

  switch (c->type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    return llist_get(c->u.llist, idx);
    break;
  default:
    goto cleanup;
  }

cleanup:
  return NULL;
}

void ct_remove(struct Container *c, const size_t idx, ct_free_item fn) {
  CLEANUP_IF_FAIL(c);

  switch (c->type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    llist_remove(c->u.llist, idx, fn);
    break;
  default:
    goto cleanup;
  }

cleanup:
  return;
}

void ct_foreach(const struct Container *c, void (*fn)(void *)) {
  CLEANUP_IF_FAIL(c && fn);

  switch (c->type) {
  case CT_NONE:
    goto cleanup;
    break;
  case CT_LLIST:
    llist_foreach(c->u.llist, fn);
    break;
  default:
    break;
  }

cleanup:
  return;
}
