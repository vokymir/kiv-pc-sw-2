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
// Return NULL on failure.
struct Code_Segment *cdsg_create(void);

// Free code segment.
void cdsg_free(struct Code_Segment *cdsg);

// Code Segment Append Byte.
// Return 1 on success, 0 on failure.
int cdsg_app_b(struct Code_Segment *cdsg, uint8_t b);

// Code Segment Append Bytes.
// Return 1 on success, 0 on failure.
int cdsg_app_bs(struct Code_Segment *cdsg, const uint8_t *bs, size_t count);

// Code Segment Append OP-code.
// Return 1 on success, 0 on failure.
int cdsg_app_op(struct Code_Segment *cdsg, uint8_t opcode);

// Code Segment Append Register.
// Return 1 on success, 0 on failure.
int cdsg_app_reg(struct Code_Segment *cdsg, uint8_t reg_code);

// Code Segment Append Immediate 32 bit value.
// Return 1 on success, 0 on failure.
int cdsg_app_imm(struct Code_Segment *cdsg, int32_t imm32b_v);

// Code Segment get size.
size_t cdsg_get_size(const struct Code_Segment *cdsg);

// Get pointer to Code Segment bytes. Read-only.
const uint8_t *cdsg_get_bytes(const struct Code_Segment *cdsg);

// Code Segment: Advance number of bytes (for 1st pass).
// Returns the offset where started, or SIZE_MAX on failure.
// WARN: Only use in 1st pass!
size_t cdsg_advance(struct Code_Segment *cdsg, size_t num_bytes);

#endif
