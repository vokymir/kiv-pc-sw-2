// test_container.c
#include "../src/container.h"
#include "../src/llist.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* helper */
static void print_int(void *data) {
  int *v = (int *)data;
  printf("%d ", *v);
}

/* custom destructor for complex object example */
struct Pair {
  char *s;
  int *n;
};
static void free_pair(void *p) {
  struct Pair *pair = p;
  if (!pair)
    return;
  if (pair->s)
    jree(pair->s);
  if (pair->n)
    jree(pair->n);
  jree(pair);
}

int main(void) {
  printf("Running container tests...\n");

  /* === create container === */
  struct Container *c = ct_create(CT_LLIST, sizeof(int));
  assert(c != NULL);
  assert(ct_count(c) == 0);

  /* === invalid create === */
  struct Container *bad = ct_create(CT_NONE, sizeof(int));
  assert(bad == NULL);

  /* === add items (ownership transfer) === */
  int *a = jalloc(sizeof(int));
  int *b = jalloc(sizeof(int));
  int *c_item = jalloc(sizeof(int));
  *a = 111;
  *b = 222;
  *c_item = 333;

  /* ct_add now returns pointer to stored item on success */
  void *res = NULL;
  res = ct_add(c, (void **)&a);
  assert(res != NULL);
  assert(a == NULL);

  res = ct_add(c, (void **)&b);
  assert(res != NULL);
  assert(b == NULL);

  res = ct_add(c, (void **)&c_item);
  assert(res != NULL);
  assert(c_item == NULL);

  assert(ct_count(c) == 3);

  /* === get and verify values === */
  int *g0 = ct_get(c, 0);
  int *g1 = ct_get(c, 1);
  int *g2 = ct_get(c, 2);
  assert(g0 && g1 && g2);
  assert(*g0 == 111);
  assert(*g1 == 222);
  assert(*g2 == 333);

  printf("Container items: ");
  for (size_t i = 0; i < ct_count(c); ++i) {
    print_int(ct_get(c, i));
  }
  printf("\n");

  /* === remove middle === */
  ct_remove(c, 1, NULL);
  assert(ct_count(c) == 2);
  assert(*(int *)ct_get(c, 0) == 111);
  assert(*(int *)ct_get(c, 1) == 333);

  /* === remove last === */
  ct_remove(c, 1, NULL);
  assert(ct_count(c) == 1);
  assert(*(int *)ct_get(c, 0) == 111);

  /* === remove remaining === */
  ct_remove(c, 0, NULL);
  assert(ct_count(c) == 0);

  /* === test from_llist transfer === */
  struct Llist *list = llist_create(sizeof(int));
  int *x = jalloc(sizeof(int));
  *x = 42;
  assert(llist_add(list, (void **)&x) != NULL);
  struct Container *c2 = ct_from_llist(&list);
  assert(c2 != NULL);
  assert(list == NULL); // ownership transferred
  assert(ct_count(c2) == 1);
  assert(*(int *)ct_get(c2, 0) == 42);
  ct_free(c2, NULL);

  /* === complex object example with custom destructor === */
  struct Container *c3 = ct_create(CT_LLIST, sizeof(void *));
  struct Pair *pair = jalloc(sizeof(*pair));
  pair->s = jalloc(8);
  strcpy(pair->s, "hi");
  pair->n = jalloc(sizeof(int));
  *pair->n = 99;

  res = ct_add(c3, (void **)&pair);
  assert(res != NULL);
  assert(pair == NULL); /* ownership moved */

  ct_free(c3, free_pair);

  /* === free original container === */
  ct_free(c, NULL);

  /* === verify memory accounting === */
  assert(jemory() == 0);
  printf("Freed all containers successfully, no leaks.\n");

  printf("✅ All container tests passed!\n");
  return 0;
}
