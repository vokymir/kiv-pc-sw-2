#ifndef PARSER_SEGMENT
#define PARSER_SEGMENT

#include "parser.h"

// Increment pstmt segment count.
// Return idx of appended segment on success, SIZE_MAX on failure.
size_t segment_append(struct Parsed_Statement *pstmt);

// Allocate space for segments based on info in pstmt.
// -- in recursion every segment say how many space it needs. when recursion
// ends (here) all space is allocated and when going back from recursion, the
// allocated space is filled. Return 1 on success, 0 on failure.
int segments_finalize(struct Parsed_Statement *pstmt);

// Count total size of data declaration based on defined segments and set it in
// <dd>->total_size. Return 1 on success, 0 on failure.
int segment_set_dd_total_size(struct Data_Declaration *dd);

// Decrement pstmt segment count on failure
void segment_remove_last(struct Parsed_Statement *pstmt);

// Set default values for DUP init segment. Requires that DUP already exist
// inside the segment. Return 1 on success, 0 on failure.
int segment_set_dup(struct Parsed_Statement *pstmt, size_t segment_idx);

// Set numeric value in segment from token.
// Return 1 on success, 0 on failure.
int segment_set_number(struct Parsed_Statement *pstmt, size_t segment_idx,
                       const struct Token *token);

// Set uninitialized value in segment from token.
// Return 1 on success, 0 on failure.
int segment_set_uninit(struct Parsed_Statement *pstmt, size_t segment_idx);

// Set string value in segment from <token>->value.
// Return 1 on success, 0 on failure.
int segment_set_string(struct Parsed_Statement *pstmt, size_t segment_idx,
                       const struct Token *token);

#endif
