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

// ===== TOKEN HELPER DECLARATIONS =====
// Check whether next <count> tokens exist - only the last one can be EOF.
// On success return 1, on failure 0.
static int _exist_tokens(const struct Token *tokens[], size_t count);

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

static int _parse_int(const struct Token *token, int *out);

// ===== SEGMENT HELPER DECLARATIONS =====

// Increment pstmt segment count
static int _append_segment(struct Parsed_Statement *pstmt);

// Allocate space for segments
static int _finalize_segments(struct Parsed_Statement *pstmt);

// Decrement pstmt segment count on failure
static void _remove_last_segment(struct Parsed_Statement *pstmt);

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
  if (_token_is_eof(tokens[0])) {
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

  pstmt->type = STMT_SECTION_CODE;
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

  strncpy(pstmt->content.label_def.label_name, tokens[0]->value,
          sizeof(pstmt->content.label_def.label_name));

  return GRM_MATCH;
}

enum Err_Grm grammar_line_identifier(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(_token_is(tokens[0], TOKEN_IDENTIFIER));

  NOMATCH_IF_FAIL(grammar_identifier_def(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;
  strncpy(pstmt->content.data_decl.identifier, tokens[0]->value,
          sizeof(pstmt->content.data_decl.identifier));

  return GRM_MATCH;
}

enum Err_Grm grammar_line_instruction(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[]) {
  struct Instruction_Statement *is = NULL;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(_token_is(tokens[0], TOKEN_LABEL));

  NOMATCH_IF_FAIL(grammar_instruction_rhs(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_INSTRUCTION;
  pstmt->err = PAR_NO_ERROR;
  is = &pstmt->content.instruction;
  is->descriptor = instruction_find(tokens[0]->value, strlen(tokens[0]->value),
                                    is->operands[0].type, is->operands[1].type);
  NOMATCH_IF_FAIL(is->descriptor);

  return GRM_MATCH;
}

enum Err_Grm grammar_identifier_def(struct Parsed_Statement *pstmt,
                                    const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(_token_is(tokens[0], TOKEN_DATA_TYPE));

  if (strcmp(tokens[0]->value, "DWORD") == 0 ||
      strcmp(tokens[0]->value, "DW") == 0) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec(pstmt, &tokens[1]) == GRM_MATCH);
    pstmt->content.data_decl.type = DATA_DWORD;
  } else if (strcmp(tokens[0]->value, "BYTE") == 0 ||
             strcmp(tokens[0]->value, "DB") == 0) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec(pstmt, &tokens[1]) == GRM_MATCH);
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
  size_t segment_idx = SIZE_MAX;
  struct Init_Segment *segment = NULL;
  const struct Token *token = NULL;
  char *end = NULL;

  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  token = tokens[0];
  segment_idx = pstmt->content.data_decl.segment_count;
  pstmt->content.data_decl.segment_count++;

  if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_DUP))) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dup(pstmt, &tokens[0]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_DUP;
    segment->element_count = 1;

  } else if (_token_is(tokens[0], TOKEN_NUMBER)) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->data.value = strtoimax(token->value, &end, 10);

    CLEANUP_IF_FAIL(token->value != end);
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 0;
    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->element_count = 1;

  } else if (_token_is(tokens[0], TOKEN_QUESTION)) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]) == GRM_MATCH);

    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 1;
    segment->element_count = 1;

  } else {
    goto cleanup;
  }

  return GRM_MATCH;

cleanup:
  if (segment_idx < SIZE_MAX) {
    pstmt->content.data_decl.segment_count--;
  }
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_dw_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (_token_is(tokens[0], TOKEN_COMMA)) {
    NOMATCH_IF_FAIL(grammar_identifier_dw_dec(pstmt, &tokens[1]) == GRM_MATCH);
  } else if (_token_is_eof(tokens[0])) {
    pstmt->content.data_decl.segments = jalloc(
        sizeof(struct Init_Segment) * pstmt->content.data_decl.segment_count);
    NOMATCH_IF_FAIL(pstmt);
  } else {
    return GRM_NO_MATCH;
  }

  return GRM_MATCH;
}

enum Err_Grm grammar_identifier_dw_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  int is_uninit = 0;
  char *end = NULL;
  size_t segment_idx = SIZE_MAX, dup_len = 5;
  struct Init_Segment *segment = NULL;

  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(
      _tokens_start_with(tokens, 3,
                         TOK_ARR(TOKEN_NUMBER, TOKEN_DUP, TOKEN_LPAREN)) &&
      _token_is(tokens[4], TOKEN_LPAREN));

  if (_token_is(tokens[3], TOKEN_NUMBER)) {
    is_uninit = 0;
  } else if (_token_is(tokens[3], TOKEN_QUESTION)) {
    is_uninit = 1;
  } else {
    goto cleanup;
  }

  segment_idx = pstmt->content.data_decl.segment_count;
  pstmt->content.data_decl.segment_count++;

  // parse next part of line
  CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[dup_len]) ==
                  GRM_MATCH);

  segment = &pstmt->content.data_decl.segments[segment_idx];
  segment->data.dup.count = strtoimax(tokens[0]->value, &end, 10);
  CLEANUP_IF_FAIL(tokens[0]->value != end);

  segment->type = INIT_SEG_DUP;
  segment->element_count = segment->data.dup.count;

  if (!is_uninit) {
    segment->data.dup.value = strtoimax(tokens[3]->value, &end, 10);
    CLEANUP_IF_FAIL(tokens[3]->value != end);

    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->is_uninit = 0;
  } else {
    segment->is_uninit = 1;
  }

  return GRM_MATCH;

cleanup:
  if (segment_idx < SIZE_MAX) {
    pstmt->content.data_decl.segment_count--;
  }
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_db_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  size_t segment_idx = SIZE_MAX;
  struct Init_Segment *segment = NULL;
  const struct Token *token = NULL;
  char *end = NULL;

  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  token = tokens[0];
  segment_idx = pstmt->content.data_decl.segment_count;
  pstmt->content.data_decl.segment_count++;

  if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_DUP))) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dup(pstmt, &tokens[0]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_DUP;
    segment->element_count = 1;

  } else if (_token_is(tokens[0], TOKEN_NUMBER)) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &tokens[1]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->data.value = strtoimax(token->value, &end, 10);

    CLEANUP_IF_FAIL(token->value != end);
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 0;
    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->element_count = 1;

  } else if (_token_is(tokens[0], TOKEN_QUESTION)) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &tokens[1]) == GRM_MATCH);

    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 1;
    segment->element_count = 1;

  } else {
    goto cleanup;
  }

  return GRM_MATCH;

cleanup:
  if (segment_idx < SIZE_MAX) {
    pstmt->content.data_decl.segment_count--;
  }
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_db_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (_token_is(tokens[0], TOKEN_COMMA)) {
    NOMATCH_IF_FAIL(grammar_identifier_db_dec(pstmt, &tokens[1]) == GRM_MATCH);
  } else if (_token_is_eof(tokens[0])) {
    pstmt->content.data_decl.segments = jalloc(
        sizeof(struct Init_Segment) * pstmt->content.data_decl.segment_count);
    NOMATCH_IF_FAIL(pstmt);
  } else {
    return GRM_NO_MATCH;
  }

  return GRM_MATCH;
}

enum Err_Grm grammar_identifier_db_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  int is_uninit = 0;
  char *end = NULL;
  size_t segment_idx = SIZE_MAX, dup_len = 5;
  struct Init_Segment *segment = NULL;

  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(
      _tokens_start_with(tokens, 3,
                         TOK_ARR(TOKEN_NUMBER, TOKEN_DUP, TOKEN_LPAREN)) &&
      _token_is(tokens[4], TOKEN_LPAREN));

  if (_token_is(tokens[3], TOKEN_NUMBER)) {
    is_uninit = 0;
  } else if (_token_is(tokens[3], TOKEN_QUESTION)) {
    is_uninit = 1;
  } else {
    goto cleanup;
  }

  segment_idx = pstmt->content.data_decl.segment_count;
  pstmt->content.data_decl.segment_count++;

  // parse next part of line
  CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &tokens[dup_len]) ==
                  GRM_MATCH);

  segment = &pstmt->content.data_decl.segments[segment_idx];
  segment->data.dup.count = strtoimax(tokens[0]->value, &end, 10);
  CLEANUP_IF_FAIL(tokens[0]->value != end);

  segment->type = INIT_SEG_DUP;
  segment->element_count = segment->data.dup.count;

  if (!is_uninit) {
    segment->data.dup.value = strtoimax(tokens[3]->value, &end, 10);
    CLEANUP_IF_FAIL(tokens[3]->value != end);

    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->is_uninit = 0;
  } else {
    segment->is_uninit = 1;
  }

  return GRM_MATCH;

cleanup:
  if (segment_idx < SIZE_MAX) {
    pstmt->content.data_decl.segment_count--;
  }
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (_token_is_eof(tokens[0])) {
    pstmt->content.instruction.operand_count = 0;
    pstmt->content.instruction.operands[0].type = OP_NONE;
    return GRM_MATCH;
  }
  NOMATCH_IF_FAIL(_exist_tokens(tokens, 2));

  if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_LABEL, TOKEN_EOF))) {
    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_IMM32;
    strncpy(pstmt->content.instruction.operands[0].value.label,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[0].value.label));
  } else if (_tokens_start_with(tokens, 2,
                                TOK_ARR(TOKEN_REGISTER, TOKEN_EOF))) {
    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_REG;
    strncpy(pstmt->content.instruction.operands[0].value.register_name,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[0].value.register_name));
  } else if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_EOF))) {
    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_IMM32;
    pstmt->content.instruction.operands[0].value.immediate_value =
        42; // TODO: strotoimax
  } else if (_tokens_start_with(tokens, 2,
                                TOK_ARR(TOKEN_REGISTER, TOKEN_COMMA))) {
    NOMATCH_IF_FAIL(grammar_instruction_rhs_after(pstmt, &tokens[1]) ==
                    GRM_MATCH);

    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_REG;
    strncpy(pstmt->content.instruction.operands[0].value.register_name,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[0].value.register_name));

  } else {
    return GRM_NO_MATCH;
  }

  return GRM_MATCH;
}

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_REGISTER, TOKEN_EOF))) {
    pstmt->content.instruction.operands[1].type = OP_REG;
    strncpy(pstmt->content.instruction.operands[1].value.register_name,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[1].value.register_name));
  } else if (_tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_EOF))) {
    pstmt->content.instruction.operands[1].type = OP_IMM32;
    pstmt->content.instruction.operands[1].value.immediate_value =
        42; // TODO: strotoimax
  } else if (_tokens_start_with(
                 tokens, 3,
                 TOK_ARR(TOKEN_OFFSET, TOKEN_IDENTIFIER, TOKEN_EOF))) {
    pstmt->content.instruction.operands[1].type = OP_OFFSET;
    strncpy(pstmt->content.instruction.operands[1].value.register_name,
            tokens[1]->value,
            sizeof(pstmt->content.instruction.operands[1].value.register_name));

  } else {
    return GRM_NO_MATCH;
  }

  return GRM_MATCH;
}

// ===== TOKEN HELPER DEFINITIONS =====

static int _exist_tokens(const struct Token *tokens[], size_t count) {
  size_t i = 0;
  if (!tokens && !*tokens) {
    return 0;
  }

  for (i = 0; i < count; i++) {
    if (i < count - 1 && tokens[i]->type == TOKEN_EOF) {
      return 0;
    }
  }

  return 1;
}

static int _token_is(const struct Token *tok, enum Token_Type type) {
  return (tok && tok->type == type);
}

static int _token_is_eof(const struct Token *tok) {
  return (tok && tok->type == TOKEN_EOF);
}

static int _tokens_start_with(const struct Token *tokens[], size_t n,
                              const enum Token_Type types[]) {
  size_t i = 0;
  if (!tokens || !*tokens) {
    return 0;
  }

  for (i = 0; i < n; i++) {
    if (!_token_is(tokens[i], types[i])) {
      return 0;
    }
  }

  return 1;
}

static int _peek_type(const struct Token *tokens[], size_t idx,
                      enum Token_Type type) {
  size_t i = 0;
  if (!tokens || !*tokens) {
    return 0;
  }

  for (i = 0; i < idx - 1; i++) {
    if (_token_is_eof(tokens[i])) {
      return 0;
    }
  }

  return _token_is(tokens[idx], type);
}

// ===== STRING HELPER DEFINITIONS =====

static int _copy_token_value(const struct Token *token, char *dest,
                             size_t len) {
  RETURN_IF_FAIL(token && dest && len > 0, 0);

  strncpy(dest, token->value, len);
  dest[len] = '\0';

  return 1;
}

static int _token_value_eq(const struct Token *token, const char *s) {
  RETURN_IF_FAIL(token && s, 0);

  return (strcmp(token->value, s) == 0);
}

// ===== NUMBER HELPER DEFINITIONS =====

static int _parse_int(const struct Token *token, int *out) {
  intmax_t res = 0;
  char *end = NULL;
  RETURN_IF_FAIL(token && out, 0);

  res = strtoimax(token->value, &end, 10);
  RETURN_IF_FAIL(token->value != end, 0);
  RETURN_IF_FAIL(res <= INT_MIN && res <= INT_MAX, 0);

  *out = (int)res;
  return 1;
}

// ===== SEGMENT HELPER DEFINITIONS =====

static int _append_segment(struct Parsed_Statement *pstmt) {
  RETURN_IF_FAIL(pstmt, 0);

  pstmt->content.data_decl.segment_count++;
  return 1;
}

static int _finalize_segments(struct Parsed_Statement *pstmt) {
  struct Data_Declaration *dd = NULL;
  RETURN_IF_FAIL(pstmt, 0);

  dd = &pstmt->content.data_decl;
  dd->segments = jalloc(dd->segment_count * sizeof(struct Init_Segment));
  RETURN_IF_FAIL(dd->segments, 0);

  return 1;
}

static void _remove_last_segment(struct Parsed_Statement *pstmt) {
  if (!pstmt) {
    return;
  }

  pstmt->content.data_decl.segment_count--;
}
