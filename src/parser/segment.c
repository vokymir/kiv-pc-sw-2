#include <string.h>

#include "common.h"
#include "memory.h"
#include "parser.h"

#include "parser_segment.h"
#include "parser_token.h"

size_t segment_append(struct Parsed_Statement *pstmt) {
  RET_STDERR_IF_FAIL(pstmt, SIZE_MAX,
                     "Tried append segment to NULL parsed statement.");

  if (pstmt->content.data_decl.segment_count == SIZE_MAX) {
    return SIZE_MAX;
  }

  // current count = next segment idx (0 vs 1 indexing)
  // return current but increment after
  return pstmt->content.data_decl.segment_count++;
}

int segments_finalize(struct Parsed_Statement *pstmt) {
  struct Data_Declaration *dd = NULL;
  RET_STDERR_IF_FAIL(pstmt, 0,
                     "Tried allocate segments for NULL parsed statement.");

  dd = &pstmt->content.data_decl;
  dd->segments = jalloc(dd->segment_count * sizeof(struct Init_Segment));
  RET_STDERR_IF_FAIL(dd->segments, 0,
                     "Couldn't allocate segments for parsed statement.");

  dd->is_fully_uninit = 1; // set this assumption, if in any segment is not
                           // true, will be rewritten

  return 1;
}

int segment_set_dd_total_size(struct Data_Declaration *dd) {
  size_t i = 0, elem_size = 0;
  struct Init_Segment *is = NULL;
  RET_STDERR_IF_FAIL(dd && dd->segments, 0,
                     "Tried set total size of data declaration on NULL.");

  elem_size = (dd->type == DATA_DWORD) ? KMA_DWORD_SIZE : KMA_BYTE_SIZE;
  dd->total_size = 0;

  for (i = 0; i < dd->segment_count; i++) {
    is = &dd->segments[i];
    dd->total_size += is->element_count * elem_size;
  }

  return 1;
}

void segment_remove_last(struct Parsed_Statement *pstmt) {
  if (!pstmt) {
    PRINT_ERR("Tried remove last segment on NULL parsed statement.");
    return;
  }

  pstmt->content.data_decl.segment_count--;
}

int segment_set_dup(struct Parsed_Statement *pstmt, size_t segment_idx) {
  struct Init_Segment *segment = NULL;
  RET_STDERR_IF_FAIL(pstmt, 0, "Tried set DUP on NULL parsed statement.");
  segment = &pstmt->content.data_decl.segments[segment_idx];

  segment->type = INIT_SEG_DUP;
  segment->element_count = segment->data.dup.count; // MUST be set before
  if (segment->is_uninit == 0) {                    // MUST be set before
    pstmt->content.data_decl.is_fully_uninit = 0;
  }

  return 1;
}

int segment_set_number(struct Parsed_Statement *pstmt, size_t segment_idx,
                       const struct Token *token) {
  struct Init_Segment *segment = NULL;
  RET_STDERR_IF_FAIL(pstmt, 0, "Tried set NUMBER on NULL parsed statement.");
  segment = &pstmt->content.data_decl.segments[segment_idx];

  RETURN_IF_FAIL(token_parse_int32(token, &segment->data.value), 0);
  segment->type = INIT_SEG_VALUE;
  segment->element_count = 1;
  segment->is_uninit = 0;
  pstmt->content.data_decl.is_fully_uninit = 0;

  return 1;
}

int segment_set_uninit(struct Parsed_Statement *pstmt, size_t segment_idx) {
  struct Init_Segment *segment = NULL;
  RET_STDERR_IF_FAIL(pstmt, 0,
                     "Tried set UNINITIALIZED on NULL parsed statement.");
  segment = &pstmt->content.data_decl.segments[segment_idx];

  segment->type = INIT_SEG_UNINIT;
  segment->element_count = 1;
  segment->is_uninit = 1;

  return 1;
}

int segment_set_string(struct Parsed_Statement *pstmt, size_t segment_idx,
                       const struct Token *token) {
  struct Init_Segment *segment = NULL;
  RET_STDERR_IF_FAIL(pstmt, 0, "Tried set STRING on NULL parsed statement.");
  segment = &pstmt->content.data_decl.segments[segment_idx];

  RETURN_IF_FAIL(token_copy_value(token, segment->data.string,
                                  sizeof(segment->data.string)),
                 0);
  segment->type = INIT_SEG_STRING;
  segment->element_count = strlen(segment->data.string);
  segment->is_uninit = 0;
  pstmt->content.data_decl.is_fully_uninit = 0;

  return 1;
}
