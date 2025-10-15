#include "../src/datastruc.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Optional helper for debugging
static void print_int(void *data) {
  int *v = (int *)data;
  printf("%d ", *v);
}

// Optional free function for items with internal pointers (not used here)
static void free_inner(void *data) {
  (void)data; // nothing to free
}

int main(void) {
  printf("Running linked list tests...\n");

  // === Create list ===
  struct DS_Llist *list = ds_llist_new(sizeof(int));
  assert(list != NULL);
  assert(list->count == 0);
  printf("Created new list.\n");

  // === Add items ===
  int a = 10, b = 20, c = 30;
  assert(ds_llist_add(list, &a) != NULL);
  assert(ds_llist_add(list, &b) != NULL);
  assert(ds_llist_add(list, &c) != NULL);
  assert(list->count == 3);
  printf("Added 3 items.\n");

  // === Get items ===
  int *p0 = (int *)ds_llist_get_data(list, 0);
  int *p1 = (int *)ds_llist_get_data(list, 1);
  int *p2 = (int *)ds_llist_get_data(list, 2);
  assert(p0 && p1 && p2);
  assert(*p0 == 10);
  assert(*p1 == 20);
  assert(*p2 == 30);
  printf("Get item test passed.\n");

  // === Foreach test ===
  printf("List content (foreach): ");
  ds_llist_foreach(list, print_int);
  printf("\n");

  // === Remove middle ===
  ds_llist_remove(list, 1, NULL);
  assert(list->count == 2);
  assert(*(int *)ds_llist_get_data(list, 0) == 10);
  assert(*(int *)ds_llist_get_data(list, 1) == 30);
  printf("Removed middle element.\n");

  // === Remove first ===
  ds_llist_remove(list, 0, NULL);
  assert(list->count == 1);
  assert(*(int *)ds_llist_get_data(list, 0) == 30);
  printf("Removed first element.\n");

  // === Remove last ===
  ds_llist_remove(list, 0, NULL);
  assert(list->count == 0);
  printf("Removed last element, list empty.\n");

  // === Free ===
  ds_llist_free(list, free_inner);
  assert(jemory() == 0);
  printf("Freed list successfully, no leaks.\n");

  printf("✅ All linked list tests passed!\n");
  return 0;
}
