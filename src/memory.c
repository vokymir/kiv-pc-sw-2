#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"

static size_t alloc_count = 0;

void *jalloc(const size_t bytes) {
  void *mem = NULL;
  if (bytes == 0 || bytes > SIZE_MAX / 1) {
    return NULL;
  }
  mem = calloc(bytes, 1);
  if (!mem) {
    return NULL;
  }
  ++alloc_count;
  return mem;
}

void *jealloc(void *src, const size_t bytes) {
  void *mem = NULL;
  if (!src) {
    return NULL;
  }
  mem = realloc(src, bytes);

  return mem;
}

void jree(void *memory) {
  if (!memory) {
    return;
  }
  assert(alloc_count > 0);
  --alloc_count;
  free(memory);
}

void jree_clear(void **memory_ptr) {
  if (!memory_ptr || !*memory_ptr) {
    return;
  }
  jree(*memory_ptr);
  *memory_ptr = NULL;
}

size_t jemory(void) { return alloc_count; }

char *jtrdup(const char *str1) {
  size_t len = 0;
  char *dup = NULL;
  if (!str1) {
    return NULL;
  }
  len = strlen(str1) + 1;
  dup = jalloc(len);
  if (dup) {
    memcpy(dup, str1, len);
  }
  return dup;
}

char *jtrndup(const char *str, size_t size) {
  char *dup = jalloc(size + 1);
  if (!dup)
    return NULL;

  memcpy(dup, str, size);
  dup[size] = '\0';
  return dup;
}
