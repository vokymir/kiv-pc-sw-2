#ifndef PARSER_SEGMENT
#define PARSER_SEGMENT

#include "parser.h"

// Increment pstmt segment count.
// Return idx of appended segment on success, SIZE_MAX on failure.
size_t segment_append(struct Parsed_Statement *pstmt);

// Allocate space for segments
int segments_finalize(struct Parsed_Statement *pstmt);

// Set total size of data declaration based on defined segments
// Return 1 on success, 0 on failure.
int segment_set_dd_total_size(struct Data_Declaration *dd);

// Decrement pstmt segment count on failure
void segment_remove_last(struct Parsed_Statement *pstmt);

// Set default values for DUP init segment. Requires the DUP already exist
// inside the segment.
int segment_set_dup(struct Parsed_Statement *pstmt, size_t segment_idx);

int segment_set_number(struct Parsed_Statement *pstmt, size_t segment_idx,
                       const struct Token *token);

int segment_set_uninit(struct Parsed_Statement *pstmt, size_t segment_idx);

int segment_set_string(struct Parsed_Statement *pstmt, size_t segment_idx,
                       const struct Token *token);

#endif
