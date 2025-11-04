#include "memory.h"
#include "parser_identifier.h"
#include "parser_segment.h"
#include "parser_token.h"

// ===== COMMON LOGIC DECLARATIONS =====

// Both grammar_identifier_DW/DB_dec do almost the same = centralized control &
// maintenance. If !is_dw then is_db.
static enum Err_Grm _grammar_identifier_dec(struct Parsed_Statement *pstmt,
                                            const struct Token *tokens[],
                                            int is_dw);

// Very similiar functionality of DW/DB. Centralized logic, switchable.
static enum Err_Grm _grammar_identifier_dec2(struct Parsed_Statement *pstmt,
                                             const struct Token *tokens[],
                                             int is_dw);

// Common logic for both DW/DB_dup.
static enum Err_Grm _grammar_identifier_dup(struct Parsed_Statement *pstmt,
                                            const struct Token *tokens[],
                                            size_t segment_idx, int is_dw);

// ===== HEADER DEFINITIONS =====

enum Err_Grm grammar_identifier_def(struct Parsed_Statement *pstmt,
                                    const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(token_is(TOK_CURR, TOKEN_DATA_TYPE));

  if (token_value_eq(TOK_CURR, "DWORD") || token_value_eq(TOK_CURR, "DW")) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    pstmt->content.data_decl.type = DATA_DWORD;
  } else if (token_value_eq(TOK_CURR, "BYTE") ||
             token_value_eq(TOK_CURR, "DB")) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    pstmt->content.data_decl.type = DATA_BYTE;
  } else {
    return GRM_NO_MATCH;
  }

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;

cleanup:
  if (pstmt->content.data_decl.segments) {
    jree(pstmt->content.data_decl.segments);
  }
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_dw_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  return _grammar_identifier_dec(pstmt, tokens, 1);
}

enum Err_Grm grammar_identifier_dw_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]) {
  return _grammar_identifier_dec2(pstmt, tokens, 1);
}

enum Err_Grm grammar_identifier_dw_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[],
                                       size_t segment_idx) {
  enum Err_Grm res = _grammar_identifier_dup(pstmt, tokens, segment_idx, 1);
  if (res != GRM_MATCH) {
    return res;
  }
  if (!segment_set_dup(pstmt, segment_idx)) {
    return GRM_GENERIC_ERROR;
  }
  return GRM_MATCH;
}

enum Err_Grm grammar_identifier_db_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  return _grammar_identifier_dec(pstmt, tokens, 0);
}

enum Err_Grm grammar_identifier_db_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]) {
  return _grammar_identifier_dec2(pstmt, tokens, 0);
}

enum Err_Grm grammar_identifier_db_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[],
                                       size_t segment_idx) {
  enum Err_Grm res = _grammar_identifier_dup(pstmt, tokens, segment_idx, 0);
  if (res != GRM_MATCH) {
    return res;
  }
  if (!segment_set_dup(pstmt, segment_idx)) {
    return GRM_GENERIC_ERROR;
  }
  return GRM_MATCH;
}

// ===== COMMON LOGIC DECLARATIONS =====

static enum Err_Grm _grammar_identifier_dec(struct Parsed_Statement *pstmt,
                                            const struct Token *tokens[],
                                            int is_dw) {
  size_t segment_idx = SIZE_MAX;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  segment_idx = segment_append(pstmt);
  NOMATCH_IF_FAIL(segment_idx != SIZE_MAX);

  if (tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_DUP))) {
    if (is_dw) {
      CLEANUP_IF_FAIL(grammar_identifier_dw_dup(pstmt, &TOK_CURR,
                                                segment_idx) == GRM_MATCH);
    } else {
      CLEANUP_IF_FAIL(grammar_identifier_db_dup(pstmt, &TOK_CURR,
                                                segment_idx) == GRM_MATCH);
    }
    return GRM_MATCH;
  } else if (token_is(TOK_CURR, TOKEN_NUMBER)) {
    if (is_dw) {
      CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    } else {
      CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    }
    CLEANUP_IF_FAIL(segment_set_number(pstmt, segment_idx, TOK_CURR));
    return GRM_MATCH;
  } else if (token_is(TOK_CURR, TOKEN_QUESTION)) {
    if (is_dw) {
      CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    } else {
      CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    }
    CLEANUP_IF_FAIL(segment_set_uninit(pstmt, segment_idx));
    return GRM_MATCH;
  } else {
    if (!is_dw && token_is(TOK_CURR, TOKEN_STRING)) {
      CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
      CLEANUP_IF_FAIL(segment_set_string(pstmt, segment_idx, TOK_CURR));
      return GRM_MATCH;
    }
  }

cleanup:
  if (segment_idx < SIZE_MAX) {
    segment_remove_last(pstmt);
  }
  return GRM_NO_MATCH;
}

enum Err_Grm _grammar_identifier_dec2(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[], int is_dw) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (token_is(TOK_CURR, TOKEN_COMMA)) {
    if (is_dw) {
      NOMATCH_IF_FAIL(grammar_identifier_dw_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    } else {
      NOMATCH_IF_FAIL(grammar_identifier_db_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    }
    return GRM_MATCH;
  } else if (token_is_eof(TOK_CURR)) {
    NOMATCH_IF_FAIL(segments_finalize(pstmt));
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
}

enum Err_Grm _grammar_identifier_dup(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[],
                                     size_t segment_idx, int is_dw) {
  int is_uninit = 0;
  size_t dup_len = 5; // how many tokens it takes to have a DUP
  struct Init_Segment *segment = NULL;

  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      token_is(tokens[0], TOKEN_NUMBER) && token_is(tokens[1], TOKEN_DUP) &&
      token_is(tokens[2], TOKEN_LPAREN) && token_is(tokens[4], TOKEN_RPAREN));

  // MATCH VALUE/QUESTION
  if (token_is(tokens[3], TOKEN_NUMBER)) {
    is_uninit = 0;
  } else if (token_is(tokens[3], TOKEN_QUESTION)) {
    is_uninit = 1;
  } else { // guard so don't descend more if no match
    return GRM_NO_MATCH;
  }

  // TILL the EOF
  if (is_dw) {
    NOMATCH_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[dup_len]) ==
                    GRM_MATCH);
  } else {
    NOMATCH_IF_FAIL(grammar_identifier_db_dec2(pstmt, &tokens[dup_len]) ==
                    GRM_MATCH);
  }
  segment = &pstmt->content.data_decl.segments[segment_idx];
  segment->is_uninit = is_uninit;

  // DUP COUNT
  NOMATCH_IF_FAIL(token_parse_size_t(TOK_CURR, &segment->data.dup.count));

  // DUP VALUE
  if (!is_uninit) {
    NOMATCH_IF_FAIL(token_parse_int32(tokens[3], &segment->data.dup.value));
  }

  return GRM_MATCH;
}
