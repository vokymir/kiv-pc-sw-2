#ifndef DATASEG_H
#define DATASEG_H

#include <stddef.h>
#include <stdint.h>

#define DTSG_INITIAL_CAPACITY 16
#define DTSG_CAPACITY_MULT 2

struct Data_Segment {
  uint8_t *bytes;  // byte buffer
  size_t size;     // currently used
  size_t capacity; // are allocated
};

// Create new Data Segment.
// Return NULL on failure.
struct Data_Segment *dtsg_create(void);

// Free Data segment.
void dtsg_free(struct Data_Segment **dtsg);

// Data Segment Append Byte.
// Return 1 on success, 0 on failure.
int dtsg_app_b(struct Data_Segment *dtsg, uint8_t b);

// Data Segment Append Bytes.
// Return 1 on success, 0 on failure.
int dtsg_app_bs(struct Data_Segment *dtsg, const uint8_t *bs, size_t count);

// Data Segment Append DWord - little endian.
// Return 1 on success, 0 on failure.
int dtsg_app_dw(struct Data_Segment *dtsg, int32_t dw);

// Data Segment Append DWord Array.
// Return 1 on success, 0 on failure.
int dtsg_app_dws(struct Data_Segment *dtsg, const int32_t *dws, size_t count);

// Data Segment Append String ended by NULL.
// Return 1 on success, 0 on failure.
int dtsg_app_str(struct Data_Segment *dtsg, const char *string);

// Data Segment Append Zeroes.
// Return 1 on success, 0 on failure.
int dtsg_app_zs(struct Data_Segment *dtsg, size_t count);

// Data Segment get size.
size_t dtsg_get_size(const struct Data_Segment *dtsg);

// Get pointer to Data Segment bytes. Read-only.
const uint8_t *dtsg_get_bytes(const struct Data_Segment *dtsg);

// Data Segment: Advance number of bytes (for 1st pass).
// Returns the offset where started, or SIZE_MAX on failure.
// WARN: Only use in 1st pass!
size_t dtsg_advance(struct Data_Segment *dtsg, size_t num_bytes);

// Data Segment: Go to the beginning of the segment.
// Useful for reseting after 1st pass.
// WARN: Only use after 1st pass!
int dtsg_begin(struct Data_Segment *dtsg);

#endif
