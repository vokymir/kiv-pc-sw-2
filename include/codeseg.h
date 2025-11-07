#ifndef CODESEG_H
#define CODESEG_H
/* Module for working with Code Segment - provide functions for creating/freeing
 * and common usecase e.g. 'Append byte to the Code Segment'. The acronym CDSG
 * standing for CoDe SeGment is used everywhere. */

#include <stddef.h>
#include <stdint.h>

// Structure holding all information about code segment. Used in conjunction
// with other module functions it provides common use functionality.
struct Code_Segment {
  uint8_t *bytes;  // Byte buffer
  size_t size;     // Currently used bytes
  size_t capacity; // Bytes allocated
};

// Create new Code Segment. CDSG allocated via this function must be freed via
// cdsg_free. Return pointer to newly allocated Code Segment on success, NULL on
// failure.
struct Code_Segment *cdsg_create(void);

// Free given Code Segment and set the pointer to NULL to avoid dangling
// pointers.
void cdsg_free(struct Code_Segment **cdsg);

// Code Segment Append Byte.
// Return 1 on success, 0 on failure.
int cdsg_app_b(struct Code_Segment *cdsg, uint8_t b);

// Code Segment Append Bytes.
// Require pointer to uint8_t array of <count> members, which will be appended
// to the codesegment. Return 1 on success, 0 on failure.
int cdsg_app_bs(struct Code_Segment *cdsg, const uint8_t *bs, size_t count);

// Code Segment Append OP-code.
// OP-code being the byte representation of an instruction.
// Return 1 on success, 0 on failure.
int cdsg_app_op(struct Code_Segment *cdsg, uint8_t opcode);

// Code Segment Append Register.
// Return 1 on success, 0 on failure.
int cdsg_app_reg(struct Code_Segment *cdsg, uint8_t reg_code);

// Code Segment Append Immediate 32 bit value.
// Return 1 on success, 0 on failure.
int cdsg_app_imm(struct Code_Segment *cdsg, int32_t imm32b_v);

// Return the number of used bytes in CDSG. If an error occurs return 0.
size_t cdsg_get_size(const struct Code_Segment *cdsg);

// Get pointer to Code Segment bytes. Read-only.
const uint8_t *cdsg_get_bytes(const struct Code_Segment *cdsg);

// Code Segment: Advance number of bytes (for 1st pass).
// Returns the offset where started, or SIZE_MAX on failure.
// Only conscious usecase is in the 1st pass when not actually writing to CDSG,
// just simulating it for retrieving the addresses.
// WARNING: Only use in 1st pass!
size_t cdsg_advance(struct Code_Segment *cdsg, size_t num_bytes);

// Code Segment: Go to the beginning of the segment.
// Usecase: after 1st pass the CDSG 'thinks' it's full, but actually it was only
// simulated using cdsg_advance. Calling this function will reset the size
// ('pointer').
// WARNING: Only use after 1st pass!
int cdsg_begin(struct Code_Segment *cdsg);
#endif
