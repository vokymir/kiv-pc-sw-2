#include <stddef.h>
#include <string.h>

#include "array.h"
#include "common.h"
#include "memory.h"

struct Array {
  char *buffer;
  size_t capacity;
  size_t count;
  size_t item_size;
};

// Creates longer arrays buffer, copy contents & replace buffer.
// Return 1 on success, 0 on failure.
static int _array_extend(struct Array *array);

struct Array *array_create(const size_t size) {
  struct Array *array = NULL;
  if (size == 0) {
    return NULL;
  }

  array = jalloc(sizeof(struct Array));
  CLEANUP_IF_FAIL(array);

  array->buffer = jalloc(ARRAY_INITIAL_LENGTH * size);
  CLEANUP_IF_FAIL(array->buffer);

  array->item_size = size;
  array->count = 0;
  array->capacity = ARRAY_INITIAL_LENGTH;

  return array;

cleanup:
  if (array) {
    jree(array);
  }
  return NULL;
}

void array_free(struct Array *array, array_free_item fn) {
  if (!array) {
    return;
  }

  while (array->count > 0) {
    array_remove(array, array->count - 1,
                 fn); // remove from the end (performance)
  }

  jree(array);
  return;
}

size_t array_count(const struct Array *array) { return array->count; }

void *array_add(struct Array *array, void **item_ptr, array_free_item fn) {
  void *dest = NULL;
  CLEANUP_IF_FAIL(array && array->buffer && item_ptr && *item_ptr);

  if (array->count + 1 >= array->capacity) {
    CLEANUP_IF_FAIL(_array_extend(array));
  }

  dest = (char *)array->buffer + array->item_size * array->count;

  memcpy(dest, *item_ptr, array->item_size);
  array->count++;

  if (fn) {
    fn(*item_ptr);
  }
  jree(*item_ptr);

  *item_ptr = NULL;

  return dest;

cleanup:
  return NULL;
}

void *array_get(const struct Array *array, const size_t idx) {
  CLEANUP_IF_FAIL(array && array->buffer && idx < array->count);

  return (char *)array->buffer + array->item_size * idx;

cleanup:
  return NULL;
}

void array_remove(struct Array *array, const size_t idx, array_free_item fn) {
  char *item = NULL;
  CLEANUP_IF_FAIL(array && array->buffer && idx < array->count);

  item = (char *)array->buffer + array->item_size * idx;

  if (fn) { // Call destructor if provided
    fn(item);
  }

  if (idx < array->count - 1) { // Shift remaining items
    memmove(item, item + array->item_size,
            (array->count - idx - 1) * array->item_size);
  }

  array->count--;
  return;

cleanup:
  return;
}

void array_foreach(struct Array *array, void (*fn)(void *)) {
  void *item = NULL;
  size_t i = 0;
  CLEANUP_IF_FAIL(array && array->buffer && fn);

  for (i = 0; i < array->count; i++) {
    item = array->buffer + array->item_size * i;
    fn(item);
  }

cleanup:
  return;
}

static int _array_extend(struct Array *array) {
  char *new_b = NULL;
  size_t new_c = 0;
  CLEANUP_IF_FAIL(array && array->buffer);

  new_b = jalloc(array->capacity * ARRAY_SCALING_FACTOR);
  CLEANUP_IF_FAIL(new_b);

  new_c = array->capacity * array->item_size;
  memcpy(new_b, array->buffer, array->item_size * array->count);

  jree(array->buffer);
  array->buffer = new_b;
  array->capacity = new_c;
  return 1;

cleanup:
  return 0;
}
