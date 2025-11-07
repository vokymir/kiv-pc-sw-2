#ifndef JEMORY_H
#define JEMORY_H
/* Module for working with memory. A wrapper around standard functions with
 * added benefit of simple memory leaks tracking. */

#include <stddef.h>

// Allocate <bytes> bytes on the heap.
// If <bytes> == 0, return NULL and not alocate.
// Returns pointer to allocated memory on success, NULL on failure.
// Every memory block allocated via this function must be freed using jree
// (otherwise wouldn't be mismatch in count of alloc/free - or a memory leak can
// happen).
void *jalloc(const size_t bytes);

// Reallocate <src>. Now will have <bytes> number of bytes.
// If <bytes> == 0, the behaviour is undefined.
// Return new pointer on success, NULL on failure.
void *jealloc(void *src, const size_t bytes);

// Free <memory> allocated via jalloc.
void jree(void *memory);

// Return how many allocations are active right now.
// Active means allocated, not yet freed memory block.
size_t jemory(void);

// My implementation of POSIX's strdup().
// Returns a pointer to a null-terminated byte string, which is a duplicate of
// the string pointed to by <str1>. On error return NULL.
// The caller must free!
char *jtrdup(const char *str1);

// My implementation of POSIX's strndup().
// Returns a pointer to a null-terminated byte string, which is a duplicate of
// the first <size> bytes of string pointed to by <str>. On error return NULL.
// The caller must free!
char *jtrndup(const char *str, size_t size);

#endif
