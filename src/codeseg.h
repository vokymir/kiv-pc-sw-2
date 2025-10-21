#ifndef CODESEG_H
#define CODESEG_H

#include <stddef.h>
#include <stdint.h>

#define CDSG_INITIAL_CAPACITY 16
#define CDSG_CAPACITY_MULT 2

struct Code_Segment {
  uint8_t *bytes;  // byte buffer
  size_t size;     // currently used
  size_t capacity; // are allocated
};

// Create new Code Segment.
// Return NULL on failure. struct Code_Segment *cdsg_create(void);
struct Code_Segment *cdsg_create(void);

// Free code segment.
void cdsg_free(struct Code_Segment *cdsg);

// Code Segment Append Byte.
// Return 1 on success, 0 on failure.
int cdsg_app_b(struct Code_Segment *cdsg, uint8_t b);

// Code Segment Append Bytes.
// Return 1 on success, 0 on failure.
int cdsg_app_bs(struct Code_Segment *cdsg, uint8_t *bs, size_t count);

// Code Segment Append DWord - little endian.
// Return 1 on success, 0 on failure.
int cdsg_app_dw(struct Code_Segment *cdsg, uint32_t dw);

// Code Segment get size.
size_t cdsg_get_size(struct Code_Segment *cdsg);

// Get pointer to Code Segment bytes. Read-only.
const uint8_t *cdsg_get_bytes(struct Code_Segment *cdsg);

// Code Segment: Reserve number of bytes (for 1st pass).
// Reserve returns the offset where reservation started, or SIZE_MAX on failure.
// WARN: Not actually reserving anything. Just mimics it, only use in 1st pass!
size_t cdsg_reserve(struct Code_Segment *cdsg, size_t num_bytes);

#endif
