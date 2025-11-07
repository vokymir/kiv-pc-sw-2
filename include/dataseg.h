#ifndef DATASEG_H
#define DATASEG_H
/* Module for working with Data Segment - provide functionality like create/free
 * and common usecases e.g. 'Append DWORD to the Data Segment'. The acronym DTSG
 * standing for DaTa SeGment is used everywhere. */

#include <stddef.h>
#include <stdint.h>

// Structure holding all information about data segment. When used in
// conjunction with other module functions provides common usecases
// functionality.
struct Data_Segment {
  uint8_t *bytes;  // Byte buffer
  size_t size;     // Currently used bytes
  size_t capacity; // Bytes allocated
};

// Create new Data Segment. DTSG allocated using this function must be freed
// using dtsg_free. Return pointer to newly allocated DTSG on success, NULL on
// failure.
struct Data_Segment *dtsg_create(void);

// Free Data segment and set the pointer to NULL.
void dtsg_free(struct Data_Segment **dtsg);

// Data Segment Append Byte.
// Return 1 on success, 0 on failure.
int dtsg_app_b(struct Data_Segment *dtsg, uint8_t b);

// Data Segment Append Byte array.
// Require pointer to uint8_t array with <count> members, which will be appended
// to the data segment. Return 1 on success, 0 on failure.
int dtsg_app_bs(struct Data_Segment *dtsg, const uint8_t *bs, size_t count);

// Data Segment Append one byte <b> <n>-times.
// Return 1 on success, 0 on failure.
int dtsg_app_b_n(struct Data_Segment *dtsg, uint8_t b, size_t n);

// Data Segment Append DWord. Expects number in big endian and convert it to
// little endian. Return 1 on success, 0 on failure.
int dtsg_app_dw(struct Data_Segment *dtsg, int32_t dw);

// Data Segment Append DWORD Array.
// <count> specifies the length of that array
// Return 1 on success, 0 on failure.
int dtsg_app_dws(struct Data_Segment *dtsg, const int32_t *dws, size_t count);

// Data Segment Append one DWord N-times.
// Return 1 on success, 0 on failure.
int dtsg_app_dw_n(struct Data_Segment *dtsg, int32_t dw, size_t n);

// Data Segment Append String.
// The source string probably should be NULL-terminated.
// HOWEVER, the \0 byte WON'T be appended!
// Return 1 on success, 0 on failure.
int dtsg_app_str(struct Data_Segment *dtsg, const char *string);

// Data Segment Append Zeroes.
// Return 1 on success, 0 on failure.
int dtsg_app_zs(struct Data_Segment *dtsg, size_t count);

// Data Segment get number of currently used bytes.
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
