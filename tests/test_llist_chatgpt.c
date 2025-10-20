// test_llist.c
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
  printf("Running linked list tests...\n");

  /* === create === */
  struct Llist *list = llist_create(sizeof(int));
  assert(list != NULL);
  assert(llist_count(list) == 0);

  /* === invalid create === */
  struct Llist *bad = llist_create(0);
  assert(bad == NULL);

  /* === add items (ownership transfer) === */
  int *a = jalloc(sizeof(int));
  *a = 10;
  int *b = jalloc(sizeof(int));
  *b = 20;
  int *c = jalloc(sizeof(int));
  *c = 30;

  assert(a && b && c);
  assert(llist_add(list, (void **)&a) != NULL);
  assert(a == NULL);
  assert(llist_add(list, (void **)&b) != NULL);
  assert(b == NULL);
  assert(llist_add(list, (void **)&c) != NULL);
  assert(c == NULL);
  assert(llist_count(list) == 3);

  /* === get items and check values === */
  int *p0 = llist_get(list, 0);
  int *p1 = llist_get(list, 1);
  int *p2 = llist_get(list, 2);
  assert(p0 && p1 && p2);
  assert(*p0 == 10 && *p1 == 20 && *p2 == 30);

  /* === foreach (visual check) === */
  printf("List (foreach): ");
  llist_foreach(list, print_int);
  printf("\n");

  /* === remove middle === */
  llist_remove(list, 1, NULL);
  assert(llist_count(list) == 2);
  assert(*(int *)llist_get(list, 0) == 10);
  assert(*(int *)llist_get(list, 1) == 30);

  /* === remove first === */
  llist_remove(list, 0, NULL);
  assert(llist_count(list) == 1);
  assert(*(int *)llist_get(list, 0) == 30);

  /* === remove last === */
  llist_remove(list, 0, NULL);
  assert(llist_count(list) == 0);

  /* === invalid index checks === */
  assert(llist_get(list, 0) == NULL);
  llist_remove(list, 0, NULL); /* should be no-op */

  /* === custom destructor for complex payloads === */
  struct Llist *list2 = llist_create(sizeof(void *));
  struct Pair *pair = jalloc(sizeof(*pair));
  pair->s = jalloc(16);
  strcpy(pair->s, "hello");
  pair->n = jalloc(sizeof(int));
  *pair->n = 7;

  assert(llist_add(list2, (void **)&pair) != NULL);
  llist_free(list2, free_pair);

  /* === free list and check for leaks === */
  llist_free(list, NULL);
  assert(jemory() == 0);

  printf("Freed list successfully, no leaks.\n");
  printf("✅ All linked list tests passed!\n");
  return 0;
}
