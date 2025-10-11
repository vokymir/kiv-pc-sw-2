#ifndef JEMORY_H
#define JEMORY_H

#include <stddef.h>

// Somewhere/Somehow allocate these bytes.
// Returns pointer on success, NULL on failure.
void *jalloc(const size_t bytes);

// Free memory allocated via jalloc.
void jree(void *memory);

// Return how many allocations are active right now.
size_t jemory();

#endif
