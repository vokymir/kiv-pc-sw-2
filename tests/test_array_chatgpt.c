// tests/test_array.c
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/array.h"
#include "../src/memory.h"

/* A simple free-counter used to assert destructor calls */
static size_t freed_count = 0;
static void free_counter(void *item) {
  (void)item;
  freed_count++;
}

/* Helper: allocate an integer on the heap (using jalloc) */
static int *make_int(int v) {
  int *p = (int *)jalloc(sizeof(int));
  assert(p != NULL);
  *p = v;
  return p;
}

/* Helper - allocate a small struct */
struct pair {
  int x;
  char name[16];
};
static struct pair *make_pair(int x, const char *name) {
  struct pair *p = (struct pair *)jalloc(sizeof(*p));
  assert(p != NULL);
  p->x = x;
  strncpy(p->name, name, sizeof(p->name) - 1);
  p->name[sizeof(p->name) - 1] = '\0';
  return p;
}

/* ---------- Tests ---------- */

static void test_create_add_get_count(void) {
  printf("test_create_add_get_count...\n");

  struct Array *a = array_create(sizeof(int));
  assert(a != NULL);
  assert(array_count(a) == 0);

  for (int i = 0; i < 10; ++i) {
    int *val = make_int(i * 2);
    void *ret = array_add(a, (void **)&val, NULL);
    assert(ret != NULL);
    /* val should have been jree'd and nulled by array_add */
    assert(val == NULL);
    assert(array_count(a) == (size_t)(i + 1));

    int *stored = (int *)array_get(a, i);
    assert(stored != NULL);
    assert(*stored == i * 2);
  }

  array_free(a, NULL);
  printf("  PASSED\n");
}

static void test_extend_and_ordering(void) {
  printf("test_extend_and_ordering...\n");

  struct Array *a = array_create(sizeof(int));
  assert(a);

  /* Add more items than a small default capacity to trigger extend.
     We don't rely on ARRAY_INITIAL_LENGTH being any particular value,
     just add many items to force extension. */
  const int N = 200;
  for (int i = 0; i < N; ++i) {
    int *v = make_int(i + 1000);
    void *ret = array_add(a, (void **)&v, NULL);
    assert(ret != NULL);
  }
  assert(array_count(a) == (size_t)N);

  /* Check a few positions for correctness */
  assert(*(int *)array_get(a, 0) == 1000);
  assert(*(int *)array_get(a, 50) == 1050);
  assert(*(int *)array_get(a, N - 1) == 1000 + (N - 1));

  array_free(a, NULL);
  printf("  PASSED\n");
}

static void test_remove_shifts_and_destructor(void) {
  printf("test_remove_shifts_and_destructor...\n");

  freed_count = 0;
  struct Array *a = array_create(sizeof(int));
  assert(a);

  /* Add 5 ints: 0..4 */
  for (int i = 0; i < 5; ++i) {
    int *v = make_int(i);
    array_add(a, (void **)&v, NULL);
  }
  assert(array_count(a) == 5);

  /* Remove middle element (index 2). Provide destructor to count calls. */
  array_remove(a, 2, free_counter);
  /* One freed item should have been counted by free_counter */
  assert(freed_count == 1);
  assert(array_count(a) == 4);

  /* Ensure shift: new index 2 should be old index 3 (value 3) */
  assert(*(int *)array_get(a, 2) == 3);
  /* And subsequent items are correct */
  assert(*(int *)array_get(a, 3) == 4);

  /* Remove all to test repeated shift+destructor via array_free */
  array_free(a, free_counter);
  /* array_free removes remaining 4 items and calls free_counter for each */
  assert(freed_count == 1 + 4);

  printf("  PASSED\n");
}

static void test_foreach_sum(void) {
  printf("test_foreach_sum...\n");

  struct Array *a = array_create(sizeof(int));
  assert(a);

  for (int i = 0; i < 10; ++i) {
    int *v = make_int(i);
    array_add(a, (void **)&v, NULL);
  }

  struct {
    int sum;
  } ctx = {.sum = 0};

  void sum_cb(void *it) {
    int *ip = (int *)it;
    ctx.sum += *ip;
  }

  array_foreach(a, sum_cb);
  /* sum of 0..9 = 45 */
  assert(ctx.sum == 45);

  array_free(a, NULL);
  printf("  PASSED\n");
}

static void test_struct_item_handling(void) {
  printf("test_struct_item_handling...\n");

  struct Array *a = array_create(sizeof(struct pair));
  assert(a);

  struct pair *p1 = make_pair(1, "alpha");
  struct pair *p2 = make_pair(2, "beta");
  struct pair *p3 = make_pair(3, "gamma");

  array_add(a, (void **)&p1, NULL);
  array_add(a, (void **)&p2, NULL);
  array_add(a, (void **)&p3, NULL);

  assert(array_count(a) == 3);

  struct pair *rp = (struct pair *)array_get(a, 1);
  assert(rp != NULL);
  assert(rp->x == 2);
  assert(strcmp(rp->name, "beta") == 0);

  array_free(a, NULL);
  printf("  PASSED\n");
}

/* Test that array_free calls destructor for each stored item */
static void test_array_free_calls_destructor(void) {
  printf("test_array_free_calls_destructor...\n");

  freed_count = 0;
  const int M = 7;
  struct Array *a = array_create(sizeof(int));
  assert(a);

  for (int i = 0; i < M; ++i) {
    int *v = make_int(i + 10);
    array_add(a, (void **)&v, NULL);
  }
  assert(array_count(a) == (size_t)M);

  array_free(a, free_counter);
  /* free_counter should have been called M times */
  assert(freed_count == (size_t)M);

  printf("  PASSED\n");
}

/* ---------- Main ---------- */
int main(void) {
  printf("\n=== Running Array Unit Tests ===\n\n");

  test_create_add_get_count();
  test_extend_and_ordering();
  test_remove_shifts_and_destructor();
  test_foreach_sum();
  test_struct_item_handling();
  test_array_free_calls_destructor();

  printf("\n=== All Array Tests Passed! ===\n\n");
  return 0;
}
