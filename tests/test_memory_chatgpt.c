#include "../src/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Print section headers
static void print_header(const char *name) { printf("\n=== %s ===\n", name); }

// --- TESTS ---

static void test_alloc_free(void) {
  print_header("test_alloc_free");

  size_t before = jemory();
  void *ptr = jalloc(64);
  assert(ptr != NULL);
  assert(jemory() == before + 1);

  jree(ptr);
  assert(jemory() == before);

  printf("✅ alloc_free passed.\n");
}

static void test_alloc_clear(void) {
  print_header("test_alloc_clear");

  void *ptr = jalloc(32);
  assert(ptr != NULL);
  assert(jemory() > 0);

  jree_clear(&ptr);
  assert(ptr == NULL);
  printf("✅ alloc_clear passed.\n");
}

static void test_realloc_grow_and_shrink(void) {
  print_header("test_realloc_grow_and_shrink");

  void *p = jalloc(16);
  assert(p);

  size_t before = jemory();

  // Grow
  void *newp = jealloc(p, 64);
  assert(newp != NULL);
  assert(jemory() == before); // count should stay same if impl replaces pointer

  // Shrink
  void *newp2 = jealloc(newp, 8);
  assert(newp2 != NULL);
  assert(jemory() == before);

  jree(newp2);
  assert(jemory() == before - 1);

  printf("✅ realloc_grow_and_shrink passed.\n");
}

static void test_str_duplicate(void) {
  print_header("test_str_duplicate");

  const char *src = "Jemory works fine!";
  char *dup = jtrdup(src);
  assert(dup != NULL);
  assert(strcmp(src, dup) == 0);

  size_t before = jemory();
  jree(dup);
  assert(jemory() == before - 1);

  printf("✅ str_duplicate passed.\n");
}

static void test_strn_duplicate(void) {
  print_header("test_strn_duplicate");

  const char *src = "abcdef";
  char *dup = jtrndup(src, 3);
  assert(dup != NULL);
  assert(strcmp(dup, "abc") == 0);

  jree(dup);
  printf("✅ strn_duplicate passed.\n");
}

static void test_multiple_allocations(void) {
  print_header("test_multiple_allocations");

  void *a = jalloc(10);
  void *b = jalloc(20);
  void *c = jalloc(30);

  assert(a && b && c);
  size_t count = jemory();
  assert(count >= 3);

  jree(a);
  jree(b);
  jree(c);

  assert(jemory() == count - 3);
  printf("✅ multiple_allocations passed.\n");
}

static void test_invalid_inputs(void) {
  print_header("test_invalid_inputs");

  // Freeing NULL should not crash
  jree(NULL);
  jree_clear(NULL);

  // jealloc with NULL given should return NULL...
  void *p = jealloc(NULL, 10);
  assert(p == NULL);
  jree(p);

  printf("✅ invalid_inputs passed.\n");
}

// --- MAIN ---

int main(void) {
  printf("Running Jemory module tests...\n");

  test_alloc_free();
  test_alloc_clear();
  test_realloc_grow_and_shrink();
  test_str_duplicate();
  test_strn_duplicate();
  test_multiple_allocations();
  test_invalid_inputs();

  printf("\nAll Jemory tests passed successfully.\n");
  return 0;
}
