#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "instruction.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "parser_code.h"
#include "parser_data.h"
#include "parser_grammar.h"

// ===== TOKEN HELPER MACROS =====
#define NOMATCH_IF_FAIL(cond) RETURN_IF_FAIL((cond), GRM_NO_MATCH)
#define TOK_ARR(...) ((const enum Token_Type[]){__VA_ARGS__})
#define TOK_CURR tokens[0]
#define TOK_NEXT tokens[1]

// ===== TOKEN HELPER DECLARATIONS =====

// Check if token is not null and its type is <type>.
static int _token_is(const struct Token *token, enum Token_Type type);

// Check if token is not null and its type is EOF.
static int _token_is_eof(const struct Token *token);

// Check if first n tokens exist are exact types.
static int _tokens_start_with(const struct Token *tokens[], size_t n,
                              const enum Token_Type types[]);

// Check if token on idx exist and have type.
static int _peek_type(const struct Token *tokens[], size_t idx,
                      enum Token_Type type);

// ===== STRING HELPER DECLARATIONS =====

// copy token->value into dest and NULL-terminates
static int _copy_token_value(const struct Token *token, char *dest, size_t len);

// Safe wrapper of strcmp(token->value, s)
static int _token_value_eq(const struct Token *token, const char *s);

// ===== NUMBER HELPER DECLARATIONS =====

static int _parse_int32(const struct Token *token, int32_t *out);

// ===== SEGMENT HELPER DECLARATIONS =====

// Increment pstmt segment count.
// Return idx of appended segment on success, SIZE_MAX on failure.
static size_t _append_segment(struct Parsed_Statement *pstmt);

// Allocate space for segments
static int _finalize_segments(struct Parsed_Statement *pstmt);

// Decrement pstmt segment count on failure
static void _remove_last_segment(struct Parsed_Statement *pstmt);

// ===== GRAMMAR HELPER DECLARATIONS =====

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

// ===== INIT HELPER DECLARATIONS =====

// Set default values for DUP init segment. Requires the DUP already exist
// inside the segment.
static int _set_segment_dup(struct Parsed_Statement *pstmt, size_t segment_idx);

static int _set_segment_number(struct Parsed_Statement *pstmt,
                               size_t segment_idx, const struct Token *token);

static int _set_segment_uninit(struct Parsed_Statement *pstmt,
                               size_t segment_idx);

static int _set_segment_string(struct Parsed_Statement *pstmt,
                               size_t segment_idx, const struct Token *token);

// ===== OPERANDS HELPER DECLARATIONS =====

// set both operands
static int _set_ops_none(struct Instruction_Statement *is);

// set is->op[idx] to label of token->value
static int _set_op_label(struct Instruction_Statement *is,
                         const struct Token *token, size_t idx);

// set is->op[idx] to register of token->value
static int _set_op_register(struct Instruction_Statement *is,
                            const struct Token *token, size_t idx);

// set is->op[idx] to number saved in string format in token->value
static int _set_op_number(struct Instruction_Statement *is,
                          const struct Token *token, size_t idx);

// set is->op[idx] to offset, token must point to identifier
static int _set_op_offset(struct Instruction_Statement *is,
                          const struct Token *token, size_t idx);

// ===== HEADER DEFINITIONS =====

enum Err_Grm grammar_line(struct Parsed_Statement *pstmt,
                          const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (grammar_line_kma(pstmt, tokens) == GRM_MATCH) {
    return GRM_MATCH;
  }
  if (grammar_line_code(pstmt, tokens) == GRM_MATCH) {
    return GRM_MATCH;
  }
  if (grammar_line_data(pstmt, tokens) == GRM_MATCH) {
    return GRM_MATCH;
  }
  if (grammar_line_label(pstmt, tokens) == GRM_MATCH) {
    return GRM_MATCH;
  }
  if (grammar_line_identifier(pstmt, tokens) == GRM_MATCH) {
    return GRM_MATCH;
  }
  if (grammar_line_instruction(pstmt, tokens) == GRM_MATCH) {
    return GRM_MATCH;
  }
  if (_token_is_eof(TOK_CURR)) {
    pstmt->type = STMT_NONE;
    pstmt->err = PAR_EMPTY_LINE;
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_kma(struct Parsed_Statement *pstmt,
                              const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_KMA, TOKEN_EOF)));

  pstmt->type = STMT_KMA;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;
}

enum Err_Grm grammar_line_code(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      _tokens_start_with(tokens, 2, TOK_ARR(TOKEN_SECTION_CODE, TOKEN_EOF)));

  pstmt->type = STMT_SECTION_CODE;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;
}

enum Err_Grm grammar_line_data(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      _tokens_start_with(tokens, 2, TOK_ARR(TOKEN_SECTION_DATA, TOKEN_EOF)));

  pstmt->type = STMT_SECTION_DATA;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;
}

enum Err_Grm grammar_line_label(struct Parsed_Statement *pstmt,
                                const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      _tokens_start_with(tokens, 2, TOK_ARR(TOKEN_LABEL, TOKEN_EOF)));

  pstmt->type = STMT_LABEL_DEF;
  pstmt->err = PAR_NO_ERROR;

  RETURN_IF_FAIL(_copy_token_value(TOK_CURR,
                                   pstmt->content.label_def.label_name,
                                   sizeof(pstmt->content.label_def.label_name)),
                 GRM_GENERIC_ERROR);

  return GRM_MATCH;
}

enum Err_Grm grammar_line_identifier(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(_token_is(TOK_CURR, TOKEN_IDENTIFIER));
  NOMATCH_IF_FAIL(grammar_identifier_def(pstmt, &TOK_NEXT) == GRM_MATCH);

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;

  RETURN_IF_FAIL(_copy_token_value(TOK_CURR,
                                   pstmt->content.data_decl.identifier,
                                   sizeof(pstmt->content.data_decl.identifier)),
                 GRM_GENERIC_ERROR);

  return GRM_MATCH;
}

enum Err_Grm grammar_line_instruction(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[]) {
  struct Instruction_Statement *is = NULL;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(_token_is(TOK_CURR, TOKEN_INSTRUCTION));

  NOMATCH_IF_FAIL(grammar_instruction_rhs(pstmt, &TOK_NEXT) == GRM_MATCH);

  pstmt->type = STMT_INSTRUCTION;
  pstmt->err = PAR_NO_ERROR;
  is = &pstmt->content.instruction;
  is->descriptor = instruction_find(TOK_CURR->value, strlen(TOK_CURR->value),
                                    is->operands[0].type, is->operands[1].type);
  RETURN_IF_FAIL(is->descriptor, GRM_GENERIC_ERROR);

  return GRM_MATCH;
}

enum Err_Grm grammar_identifier_def(struct Parsed_Statement *pstmt,
                                    const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(_token_is(TOK_CURR, TOKEN_DATA_TYPE));

  if (_token_value_eq(TOK_CURR, "DWORD") || _token_value_eq(TOK_CURR, "DW")) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    pstmt->content.data_decl.type = DATA_DWORD;
  } else if (_token_value_eq(TOK_CURR, "BYTE") ||
             _token_value_eq(TOK_CURR, "DB")) {
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
  return _grammar_identifier_dup(pstmt, tokens, segment_idx, 1);
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
  return _grammar_identifier_dup(pstmt, tokens, segment_idx, 0);
}

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  struct Instruction_Statement *is = NULL;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  is = &pstmt->content.instruction;

  if (_token_is_eof(TOK_CURR)) {
    RETURN_IF_FAIL(_set_ops_none(is), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_LABEL, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_label(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (_tokens_start_with(tokens, 2,
                                TOK_ARR(TOKEN_REGISTER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_register(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_number(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (_tokens_start_with(tokens, 2,
                                TOK_ARR(TOKEN_REGISTER, TOKEN_COMMA))) {
    NOMATCH_IF_FAIL(grammar_instruction_rhs_after(pstmt, &tokens[2]) ==
                    GRM_MATCH);
    RETURN_IF_FAIL(_set_op_register(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
}

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]) {
  struct Operand *operand = NULL;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  operand = &pstmt->content.instruction.operands[1];

  if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_REGISTER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_register(&pstmt->content.instruction, TOK_CURR, 1),
                   GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_number(&pstmt->content.instruction, TOK_CURR, 1),
                   GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (_tokens_start_with(
                 tokens, 3,
                 TOK_ARR(TOKEN_OFFSET, TOKEN_IDENTIFIER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_offset(&pstmt->content.instruction, tokens[1], 1),
                   GRM_GENERIC_ERROR);
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
}

// ===== TOKEN HELPER DEFINITIONS =====

static int _token_is(const struct Token *tok, enum Token_Type type) {
  return (tok && tok->type == type);
}

static int _token_is_eof(const struct Token *tok) {
  return (tok && tok->type == TOKEN_EOF);
}

static int _tokens_start_with(const struct Token *tokens[], size_t n,
                              const enum Token_Type types[]) {
  size_t i = 0;
  if (!tokens || !*tokens || !types) {
    return 0;
  }

  for (i = 0; i < n; i++) {
    if (!_token_is(tokens[i], types[i])) {
      return 0;
    }
    if (i < n - 1 && _token_is_eof(tokens[i])) {
      return 0; // don't go past the end of array
    }
  }

  return 1;
}

static int _peek_type(const struct Token *tokens[], size_t idx,
                      enum Token_Type type) {
  size_t i = 0;
  if (!tokens || !*tokens)
    return 0;

  for (i = 0; i < idx; i++) {
    if (_token_is_eof(tokens[i]))
      return 0;
  }

  return _token_is(tokens[idx], type);
}

// ===== STRING HELPER DEFINITIONS =====

static int _copy_token_value(const struct Token *token, char *dest,
                             size_t len) {
  RETURN_IF_FAIL(token && dest && len > 0, 0);

  strncpy(dest, token->value, len - 1);
  dest[len - 1] = '\0';

  return 1;
}

static int _token_value_eq(const struct Token *token, const char *s) {
  RETURN_IF_FAIL(token && s, 0);

  return (strcmp(token->value, s) == 0);
}

// ===== NUMBER HELPER DEFINITIONS =====

static int _parse_int32(const struct Token *token, int32_t *out) {
  intmax_t res = 0;
  char *end = NULL;
  RETURN_IF_FAIL(token && out, 0);

  errno = 0;
  res = strtoimax(token->value, &end, 10);
  if (end == token->value)
    return 0; // no digits parsed
  if (errno == ERANGE)
    return 0; // out of range
  if (res < INT32_MIN || res > INT32_MAX)
    return 0;

  *out = (int32_t)res;
  return 1;
}

// ===== SEGMENT HELPER DEFINITIONS =====

static size_t _append_segment(struct Parsed_Statement *pstmt) {
  RETURN_IF_FAIL(pstmt, SIZE_MAX);

  if (pstmt->content.data_decl.segment_count == SIZE_MAX) {
    return SIZE_MAX;
  }

  // current count = next segment idx (0 vs 1 indexing)
  // return current but increment after
  return pstmt->content.data_decl.segment_count++;
}

static int _finalize_segments(struct Parsed_Statement *pstmt) {
  struct Data_Declaration *dd = NULL;
  RETURN_IF_FAIL(pstmt, 0);

  dd = &pstmt->content.data_decl;
  dd->segments = jalloc(dd->segment_count * sizeof(struct Init_Segment));
  RETURN_IF_FAIL(dd->segments, 0);

  dd->is_fully_uninit = 1; // set this assumption, if in any segment is not
                           // true, will be rewritten

  return 1;
}

static void _remove_last_segment(struct Parsed_Statement *pstmt) {
  if (!pstmt) {
    return;
  }

  pstmt->content.data_decl.segment_count--;
}

// ===== GRAMMAR HELPER DECLARATIONS =====

static enum Err_Grm _grammar_identifier_dec(struct Parsed_Statement *pstmt,
                                            const struct Token *tokens[],
                                            int is_dw) {
  size_t segment_idx = SIZE_MAX;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  segment_idx = _append_segment(pstmt);
  NOMATCH_IF_FAIL(segment_idx != SIZE_MAX);

  if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_DUP))) {
    if (is_dw) {
      CLEANUP_IF_FAIL(grammar_identifier_dw_dup(pstmt, &TOK_CURR,
                                                segment_idx) == GRM_MATCH);
    } else {
      CLEANUP_IF_FAIL(grammar_identifier_db_dup(pstmt, &TOK_CURR,
                                                segment_idx) == GRM_MATCH);
    }
    CLEANUP_IF_FAIL(_set_segment_dup(pstmt, segment_idx));
    return GRM_MATCH;
  } else if (_token_is(TOK_CURR, TOKEN_NUMBER)) {
    if (is_dw) {
      CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    } else {
      CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    }
    CLEANUP_IF_FAIL(_set_segment_number(pstmt, segment_idx, TOK_CURR));
    return GRM_MATCH;
  } else if (_token_is(TOK_CURR, TOKEN_QUESTION)) {
    if (is_dw) {
      CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    } else {
      CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
    }
    CLEANUP_IF_FAIL(_set_segment_uninit(pstmt, segment_idx));
    return GRM_MATCH;
  } else {
    if (!is_dw && _token_is(TOK_CURR, TOKEN_STRING)) {
      CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &TOK_NEXT) ==
                      GRM_MATCH);
      CLEANUP_IF_FAIL(_set_segment_string(pstmt, segment_idx, TOK_CURR));
      return GRM_MATCH;
    }
  }

cleanup:
  if (segment_idx < SIZE_MAX) {
    _remove_last_segment(pstmt);
  }
  return GRM_NO_MATCH;
}

enum Err_Grm _grammar_identifier_dec2(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[], int is_dw) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (_token_is(TOK_CURR, TOKEN_COMMA)) {
    if (is_dw) {
      NOMATCH_IF_FAIL(grammar_identifier_dw_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    } else {
      NOMATCH_IF_FAIL(grammar_identifier_db_dec(pstmt, &TOK_NEXT) == GRM_MATCH);
    }
    return GRM_MATCH;
  } else if (_token_is_eof(TOK_CURR)) {
    NOMATCH_IF_FAIL(_finalize_segments(pstmt));
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
}

enum Err_Grm _grammar_identifier_dup(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[],
                                     size_t segment_idx, int is_dw) {
  int is_uninit = 0;
  size_t dup_len = 5;
  struct Init_Segment *segment = NULL;

  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      _token_is(tokens[0], TOKEN_NUMBER) && _token_is(tokens[1], TOKEN_DUP) &&
      _token_is(tokens[2], TOKEN_LPAREN) && _token_is(tokens[4], TOKEN_RPAREN));

  // MATCH VALUE/QUESTION
  if (_token_is(tokens[3], TOKEN_NUMBER)) {
    is_uninit = 0;
  } else if (_token_is(tokens[3], TOKEN_QUESTION)) {
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

  // DUP COUNT
  NOMATCH_IF_FAIL(_parse_int32(TOK_CURR, &segment->data.dup.count));

  // DUP VALUE
  if (!is_uninit) {
    NOMATCH_IF_FAIL(_parse_int32(tokens[3], &segment->data.dup.value));
  }

  return GRM_MATCH;
}

// ===== INIT HELPER DECLARATIONS =====

static int _set_segment_dup(struct Parsed_Statement *pstmt,
                            size_t segment_idx) {
  struct Init_Segment *segment = NULL;
  RETURN_IF_FAIL(pstmt, 0);
  segment = &pstmt->content.data_decl.segments[segment_idx];

  segment->type = INIT_SEG_DUP;
  segment->element_count = segment->data.dup.count; // MUST be set before
  if (segment->is_uninit == 0) {                    // MUST be set before
    pstmt->content.data_decl.is_fully_uninit = 0;
  }

  return 1;
}

static int _set_segment_number(struct Parsed_Statement *pstmt,
                               size_t segment_idx, const struct Token *token) {
  struct Init_Segment *segment = NULL;
  RETURN_IF_FAIL(pstmt, 0);
  segment = &pstmt->content.data_decl.segments[segment_idx];

  RETURN_IF_FAIL(_parse_int32(token, &segment->data.value), 0);
  segment->type = INIT_SEG_VALUE;
  segment->element_count = 1;
  segment->is_uninit = 0;
  pstmt->content.data_decl.is_fully_uninit = 0;

  return 1;
}

static int _set_segment_uninit(struct Parsed_Statement *pstmt,
                               size_t segment_idx) {
  struct Init_Segment *segment = NULL;
  RETURN_IF_FAIL(pstmt, 0);
  segment = &pstmt->content.data_decl.segments[segment_idx];

  segment->type = INIT_SEG_UNINIT;
  segment->element_count = 1;
  segment->is_uninit = 1;

  return 1;
}

static int _set_segment_string(struct Parsed_Statement *pstmt,
                               size_t segment_idx, const struct Token *token) {
  struct Init_Segment *segment = NULL;
  RETURN_IF_FAIL(pstmt, 0);
  segment = &pstmt->content.data_decl.segments[segment_idx];

  RETURN_IF_FAIL(_copy_token_value(token, segment->data.string,
                                   sizeof(segment->data.string)),
                 0);
  segment->type = INIT_SEG_STRING;
  segment->element_count = strlen(segment->data.string);
  segment->is_uninit = 0;
  pstmt->content.data_decl.is_fully_uninit = 0;

  return 1;
}

// ===== OPERANDS HELPER DECLARATIONS =====

// set both operands
static int _set_ops_none(struct Instruction_Statement *is) {
  RETURN_IF_FAIL(is, 0);

  is->operand_count = 0;
  is->operands[0].type = OP_NONE;
  is->operands[1].type = OP_NONE;

  return 1;
}

// set is->op[idx] to label of token->value
static int _set_op_label(struct Instruction_Statement *is,
                         const struct Token *token, size_t idx) {
  RETURN_IF_FAIL(
      is && token && idx < sizeof(is->operands) / sizeof(struct Operand), 0);
  RETURN_IF_FAIL(_copy_token_value(token, is->operands[idx].value.label,
                                   sizeof(is->operands[idx].value.label)),
                 0);
  if (idx + 1 > is->operand_count) {
    is->operand_count = idx + 1;
  }
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_LABEL;

  return 1;
}

// set is->op[idx] to register of token->value
static int _set_op_register(struct Instruction_Statement *is,
                            const struct Token *token, size_t idx) {
  RETURN_IF_FAIL(
      is && token && idx < sizeof(is->operands) / sizeof(struct Operand), 0);
  RETURN_IF_FAIL(
      _copy_token_value(token, is->operands[idx].value.register_name,
                        sizeof(is->operands[idx].value.register_name)),
      0);
  if (idx + 1 > is->operand_count) {
    is->operand_count = idx + 1;
  }
  is->operands[idx].type = OP_REG;
  is->operands[idx].specifier = OPS_NONE;

  return 1;
}

// set is->op[idx] to number saved in string format in token->value
static int _set_op_number(struct Instruction_Statement *is,
                          const struct Token *token, size_t idx) {
  RETURN_IF_FAIL(
      is && token && idx < sizeof(is->operands) / sizeof(struct Operand), 0);
  RETURN_IF_FAIL(_parse_int32(token, &is->operands[idx].value.immediate_value),
                 0);
  if (idx + 1 > is->operand_count) {
    is->operand_count = idx + 1;
  }
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_NONE;

  return 1;
}

static int _set_op_offset(struct Instruction_Statement *is,
                          const struct Token *token, size_t idx) {
  RETURN_IF_FAIL(
      is && token && idx < sizeof(is->operands) / sizeof(struct Operand), 0);
  RETURN_IF_FAIL(_copy_token_value(token, is->operands[idx].value.label,
                                   sizeof(is->operands[idx].value.label)),
                 0);
  if (idx + 1 > is->operand_count) {
    is->operand_count = idx + 1;
  }
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_OFFSET;

  return 1;
}
