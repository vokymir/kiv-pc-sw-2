#include <stddef.h>
#include <stdlib.h>

#include "memory.h"

static size_t alloc_count = 0;

void *jalloc(const size_t bytes) {
  void *mem;
  if (bytes == 0) {
    return NULL;
  }
  mem = calloc(bytes, 1);
  if (!mem) {
    return NULL;
  }
  ++alloc_count;
  return mem;
}

void jree(void *memory) {
  if (!memory) {
    return;
  }
  --alloc_count;
  free(memory);
}

size_t jemory(void) { return alloc_count; }
