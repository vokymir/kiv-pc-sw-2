#ifndef ARRAY_H
#define ARRAY_H
// DYNAMIC LENGTH ARRAY

#include <stddef.h>

#define ARRAY_INITIAL_LENGTH 20
#define ARRAY_SCALING_FACTOR 2.0

// Struct holding all information about array.
// Opaque structure, must use functions.
struct Array;

// Function to free complex data structures saved in the array.
// DON'T FREE ITEM itself, only its insides - its used in a way that this
// behaviour would be double-free.
typedef void (*array_free_item)(void *);

// Create new array of items. One item has its <size>.
// Return pointer to array on success, NULL on failure.
struct Array *array_create(const size_t size);

// Free the whole array. If it contains complex items, provide a function which
// frees the structures (anything using pointer).
void array_free(struct Array *array, array_free_item fn);

// Return how many items are in array.
// Return 0 on failure, or if array is empty.
size_t array_count(const struct Array *array);

// Add new item to the array. Copies the item from *item_ptr to new destination.
// If item is more complex, provide free function for it. Transfers ownership to
// array by clearing the pointer (only on success). Return pointer to new item,
// NULL on failure.
void *array_add(struct Array *array, void **item_ptr, array_free_item fn);

// Get item on index from array.
// Return NULL on failure.
void *array_get(const struct Array *array, const size_t idx);

// Remove one item in the array at idx. If previous item needs special freeing,
// provide function.
void array_remove(struct Array *array, const size_t idx, array_free_item fn);

// Iterate over all array items & perform function passed as fn.
void array_foreach(struct Array *array, void (*fn)(void *));

#endif
