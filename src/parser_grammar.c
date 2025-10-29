#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "instruction.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "parser_data.h"
#include "parser_grammar.h"

// Check whether next <count> tokens exist - only the last one can be EOF.
// On success return 1, on failure 0.
static int _exist_tokens(const struct Token *tokens[], size_t count);

// Check if token is not null and its type is <type>.
static int _tok_is(const struct Token *tok, enum Token_Type type);

// Check if token is not null and its type is EOF.
static int _tok_is_eof(const struct Token *tok);

// Check if first n tokens exist are exact types.
static int _tok_start_with(const struct Token *tokens[], size_t n,
                           const enum Token_Type types[]);

// Check if token on idx exist and have type.
static int _peek_type(const struct Token *tokens[], size_t idx,
                      enum Token_Type type);

enum Err_Grm grammar_line(struct Parsed_Statement *pstmt,
                          const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

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
  if (grammar_eof(pstmt, tokens) == GRM_MATCH) {
    pstmt->type = STMT_NONE;
    pstmt->err = PAR_EMPTY_LINE;
    return GRM_MATCH;
  }

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_kma(struct Parsed_Statement *pstmt,
                              const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_KMA);

  CLEANUP_IF_FAIL(grammar_eof(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_KMA;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_code(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_SECTION_CODE);

  CLEANUP_IF_FAIL(grammar_eof(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_SECTION_CODE;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_data(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_SECTION_DATA);

  CLEANUP_IF_FAIL(grammar_eof(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_SECTION_DATA;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_label(struct Parsed_Statement *pstmt,
                                const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_LABEL);

  CLEANUP_IF_FAIL(grammar_eof(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_LABEL_DEF;
  pstmt->err = PAR_NO_ERROR;

  strncpy(pstmt->content.label_def.label_name, tokens[0]->value,
          sizeof(pstmt->content.label_def.label_name));

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_identifier(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_IDENTIFIER);

  CLEANUP_IF_FAIL(grammar_identifier_def(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;
  strncpy(pstmt->content.data_decl.identifier, tokens[0]->value,
          sizeof(pstmt->content.data_decl.identifier));

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_instruction(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_LABEL);
  CLEANUP_IF_FAIL(grammar_instruction_rhs(pstmt, &tokens[1]) == GRM_MATCH);

  pstmt->type = STMT_INSTRUCTION;
  pstmt->err = PAR_NO_ERROR;
  pstmt->content.instruction.descriptor =
      instruction_find(tokens[0]->value, strlen(tokens[0]->value),
                       pstmt->content.instruction.operands[0].type,
                       pstmt->content.instruction.operands[1].type);
  CLEANUP_IF_FAIL(pstmt->content.instruction.descriptor);

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_def(struct Parsed_Statement *pstmt,
                                    const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_DATA_TYPE);

  if (strcmp(tokens[0]->value, "DWORD") == 0 ||
      strcmp(tokens[0]->value, "DW") == 0) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec(pstmt, &tokens[1]) == GRM_MATCH);
    pstmt->content.data_decl.type = DATA_DWORD;
  } else if (strcmp(tokens[0]->value, "BYTE") == 0 ||
             strcmp(tokens[0]->value, "DB") == 0) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec(pstmt, &tokens[1]) == GRM_MATCH);
    pstmt->content.data_decl.type = DATA_BYTE;
  } else {
    goto cleanup;
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

  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);
  CLEANUP_IF_FAIL(_exist_tokens(tokens, 2)); // checking tokens[1]->type

  token = tokens[0];
  segment_idx = pstmt->content.data_decl.segment_count;
  pstmt->content.data_decl.segment_count++;

  if (token->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_DUP) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dup(pstmt, &tokens[0]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_DUP;
    segment->element_count = 1;
  } else if (token->type == TOKEN_NUMBER) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->data.value = strtoimax(token->value, &end, 10);

    CLEANUP_IF_FAIL(token->value != end);
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 0;
    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->element_count = 1;
  } else if (token->type == TOKEN_QUESTION) {
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
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  if (tokens[0]->type == TOKEN_COMMA) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec(pstmt, &tokens[1]) == GRM_MATCH);
  } else if (tokens[0]->type == TOKEN_EOF) {
    pstmt->content.data_decl.segments = jalloc(
        sizeof(struct Init_Segment) * pstmt->content.data_decl.segment_count);
    CLEANUP_IF_FAIL(pstmt);
  } else {
    goto cleanup;
  }

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_dw_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  int is_uninit = 0;
  char *end = NULL;
  size_t segment_idx = SIZE_MAX, dup_len = 5;
  struct Init_Segment *segment = NULL;

  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);
  CLEANUP_IF_FAIL(_exist_tokens(tokens, dup_len));

  CLEANUP_IF_FAIL(
      tokens[0]->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_DUP &&
      tokens[2]->type == TOKEN_LPAREN && tokens[4]->type == TOKEN_RPAREN);

  if (tokens[3]->type == TOKEN_NUMBER) {
    is_uninit = 0;
  } else if (tokens[3]->type == TOKEN_QUESTION) {
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

  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);
  CLEANUP_IF_FAIL(_exist_tokens(tokens, 2)); // checking tokens[1]->type

  token = tokens[0];
  segment_idx = pstmt->content.data_decl.segment_count;
  pstmt->content.data_decl.segment_count++;

  if (token->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_DUP) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dup(pstmt, &tokens[0]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_DUP;
  } else if (token->type == TOKEN_NUMBER) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &tokens[1]) == GRM_MATCH);
    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->data.value = strtoimax(token->value, &end, 10);

    CLEANUP_IF_FAIL(token->value != end);
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 0;
    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->element_count = 1;
  } else if (token->type == TOKEN_QUESTION) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec2(pstmt, &tokens[1]) == GRM_MATCH);

    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_UNINIT;
    segment->is_uninit = 1;
    segment->element_count = 1;
  } else if (token->type == TOKEN_STRING) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]) == GRM_MATCH);

    segment = &pstmt->content.data_decl.segments[segment_idx];
    segment->type = INIT_SEG_STRING;
    segment->is_uninit = 0;
    pstmt->content.data_decl.is_fully_uninit = 0;
    strcpy(segment->data.string, token->value);
    segment->element_count = strlen(segment->data.string);
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
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  if (tokens[0]->type == TOKEN_COMMA) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec(pstmt, &tokens[1]) == GRM_MATCH);
  } else if (tokens[0]->type == TOKEN_EOF) {
    pstmt->content.data_decl.segments = jalloc(
        sizeof(struct Init_Segment) * pstmt->content.data_decl.segment_count);
    CLEANUP_IF_FAIL(pstmt);
  } else {
    goto cleanup;
  }

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_db_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  int is_uninit = 0;
  char *end = NULL;
  size_t segment_idx = SIZE_MAX, dup_len = 5;
  struct Init_Segment *segment = NULL;

  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);
  CLEANUP_IF_FAIL(_exist_tokens(tokens, dup_len));

  CLEANUP_IF_FAIL(
      tokens[0]->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_DUP &&
      tokens[2]->type == TOKEN_LPAREN && tokens[4]->type == TOKEN_RPAREN);

  if (tokens[3]->type == TOKEN_NUMBER) {
    is_uninit = 0;
  } else if (tokens[3]->type == TOKEN_QUESTION) {
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
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  if (tokens[0]->type == TOKEN_EOF) {
    pstmt->content.instruction.operand_count = 0;
    pstmt->content.instruction.operands[0].type = OP_NONE;
    goto success;
  }
  CLEANUP_IF_FAIL(_exist_tokens(tokens, 2));

  if (tokens[0]->type == TOKEN_LABEL && tokens[1]->type == TOKEN_EOF) {
    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_IMM32;
    strncpy(pstmt->content.instruction.operands[0].value.label,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[0].value.label));
  } else if (tokens[0]->type == TOKEN_REGISTER &&
             tokens[1]->type == TOKEN_EOF) {
    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_REG;
    strncpy(pstmt->content.instruction.operands[0].value.register_name,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[0].value.register_name));
  } else if (tokens[0]->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_EOF) {
    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_IMM32;
    pstmt->content.instruction.operands[0].value.immediate_value =
        42; // TODO: strotoimax
  } else if (tokens[0]->type == TOKEN_REGISTER &&
             tokens[1]->type == TOKEN_COMMA) {
    CLEANUP_IF_FAIL(grammar_instruction_rhs_after(pstmt, &tokens[1]) ==
                    GRM_MATCH);

    pstmt->content.instruction.operand_count = 1;
    pstmt->content.instruction.operands[0].type = OP_REG;
    strncpy(pstmt->content.instruction.operands[0].value.register_name,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[0].value.register_name));

  } else {
    goto cleanup;
  }

success:
  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(_exist_tokens(tokens, 2));

  if (tokens[0]->type == TOKEN_REGISTER && tokens[1]->type == TOKEN_EOF) {
    pstmt->content.instruction.operands[1].type = OP_REG;
    strncpy(pstmt->content.instruction.operands[1].value.register_name,
            tokens[0]->value,
            sizeof(pstmt->content.instruction.operands[1].value.register_name));
  } else if (tokens[0]->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_EOF) {
    pstmt->content.instruction.operands[1].type = OP_IMM32;
    pstmt->content.instruction.operands[1].value.immediate_value =
        42; // TODO: strotoimax
  } else if (tokens[0]->type == TOKEN_OFFSET &&
             tokens[1]->type == TOKEN_IDENTIFIER) {
    CLEANUP_IF_FAIL(_exist_tokens(tokens, 3) && tokens[2]->type == TOKEN_EOF);

    pstmt->content.instruction.operands[1].type = OP_OFFSET;
    strncpy(pstmt->content.instruction.operands[1].value.register_name,
            tokens[1]->value,
            sizeof(pstmt->content.instruction.operands[1].value.register_name));

  } else {
    goto cleanup;
  }

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

// Return 1 if the token is EOF.
enum Err_Grm grammar_eof(const struct Parsed_Statement *pstmt,
                         const struct Token *tokens[]) {

  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);
  return (tokens[0]->type == TOKEN_EOF) ? GRM_MATCH : GRM_NO_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

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

static int _tok_is(const struct Token *tok, enum Token_Type type) {
  return (tok && tok->type == type);
}

static int _tok_is_eof(const struct Token *tok) {
  return (tok && tok->type == TOKEN_EOF);
}

static int _tok_start_with(const struct Token *tokens[], size_t n,
                           const enum Token_Type types[]) {
  size_t i = 0;
  if (!tokens && !*tokens) {
    return 0;
  }

  for (i = 0; i < n; i++) {
    if (!_tok_is(tokens[i], types[i])) {
      return 0;
    }
  }

  return 1;
}

static int _peek_type(const struct Token *tokens[], size_t idx,
                      enum Token_Type type) {
  size_t i = 0;
  if (!tokens && !*tokens) {
    return 0;
  }

  for (i = 0; i < idx - 1; i++) {
    if (_tok_is_eof(tokens[i])) {
      return 0;
    }
  }

  return _tok_is(tokens[idx], type);
}
