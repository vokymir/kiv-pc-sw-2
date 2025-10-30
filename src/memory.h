#ifndef JEMORY_H
#define JEMORY_H

#include <stddef.h>

// Somewhere/Somehow allocate these bytes.
// If bytes == 0, return NULL and not alocate.
// Returns pointer on success, NULL on failure.
void *jalloc(const size_t bytes);

// Reallocate src. Now will have <bytes> number of bytes.
// If bytes == 0, the behaviour is undefined.
// Return new pointer on success, NULL on failure.
void *jealloc(void *src, const size_t bytes);

// Free memory allocated via jalloc.
void jree(void *memory);

// Free memory allocated via jalloc and
// set the pointer to NULL.
void jree_clear(void **memory_ptr);

// Return how many allocations are active right now.
size_t jemory(void);

// My implementation of POSIX's strdup().
// Returns a pointer to a null-terminated byte string, which is a duplicate of
// the string pointed to by str1. On error return NULL.
// The caller must free!
char *jtrdup(const char *str1);

// My implementation of POSIX's strndup().
// Returns a pointer to a null-terminated byte string, which is a duplicate of
// the first <size> bytes of string pointed to by str. On error return NULL.
// The caller must free!
char *jtrndup(const char *str, size_t size);

#endif
