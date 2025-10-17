// test_llist.c
#include "../src/datastruc.h"
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

  /* create */
  struct DS_Llist *list = ds_llist_create(sizeof(int));
  assert(list != NULL);
  assert(list->count == 0);

  /* invalid create */
  struct DS_Llist *bad = ds_llist_create(0);
  assert(bad == NULL);

  /* === add items: allocate on heap and transfer ownership === */
  int *a = jalloc(sizeof(int));
  *a = 10;
  int *b = jalloc(sizeof(int));
  *b = 20;
  int *c = jalloc(sizeof(int));
  *c = 30;

  assert(a && b && c);

  /* ownership transfer: after add, local pointer must be NULL */
  assert(ds_llist_add(list, (void **)&a) != NULL);
  assert(a == NULL);
  assert(ds_llist_add(list, (void **)&b) != NULL);
  assert(b == NULL);
  assert(ds_llist_add(list, (void **)&c) != NULL);
  assert(c == NULL);
  assert(list->count == 3);

  /* === get items and check values === */
  int *p0 = ds_llist_get_data(list, 0);
  int *p1 = ds_llist_get_data(list, 1);
  int *p2 = ds_llist_get_data(list, 2);
  assert(p0 && p1 && p2);
  assert(*p0 == 10 && *p1 == 20 && *p2 == 30);

  /* === foreach (visual) === */
  printf("List (foreach): ");
  ds_llist_foreach(list, print_int);
  printf("\n");

  /* === remove middle === */
  ds_llist_remove(list, 1, NULL); /* will jree() the item at idx 1 */
  assert(list->count == 2);
  assert(*(int *)ds_llist_get_data(list, 0) == 10);
  assert(*(int *)ds_llist_get_data(list, 1) == 30);

  /* === remove first === */
  ds_llist_remove(list, 0, NULL);
  assert(list->count == 1);
  assert(*(int *)ds_llist_get_data(list, 0) == 30);

  /* === remove last === */
  ds_llist_remove(list, 0, NULL);
  assert(list->count == 0);

  /* === invalid index checks === */
  assert(ds_llist_get_data(list, 0) == NULL);
  ds_llist_remove(list, 0, NULL); /* should be no-op */

  /* === test custom destructor for complex payloads === */
  struct DS_Llist *list2 = ds_llist_create(sizeof(void *));
  struct Pair *pair = jalloc(sizeof(*pair));
  pair->s = jalloc(16);
  strcpy(pair->s, "hello");
  pair->n = jalloc(sizeof(int));
  *pair->n = 7;
  assert(ds_llist_add(list2, (void **)&pair) != NULL);
  /* free with custom destructor */
  ds_llist_free(list2, free_pair);

  /* === free list and check memory tracker === */
  ds_llist_free(list, NULL);

  assert(jemory() == 0); /* your allocator must track all jalloc/jree */
  printf("Freed list successfully, no leaks.\n");

  printf("✅ All linked list tests passed!\n");
  return 0;
}
