#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "grammar.h"
#include "instruction.h"
#include "lexer.h"
#include "parser.h"

#include "identifier.h"
#include "instruction.h"
#include "segment.h"
#include "token.h"

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

// set is->op_count to idx+1 if possible.
static int _set_op_count(struct Instruction_Statement *is, size_t idx);

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
  if (token_is_eof(TOK_CURR)) {
    pstmt->type = STMT_NONE;
    pstmt->err = PAR_EMPTY_LINE;
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
}

enum Err_Grm grammar_line_kma(struct Parsed_Statement *pstmt,
                              const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(tokens_start_with(tokens, 2, TOK_ARR(TOKEN_KMA, TOKEN_EOF)));

  pstmt->type = STMT_KMA;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;
}

enum Err_Grm grammar_line_code(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      tokens_start_with(tokens, 2, TOK_ARR(TOKEN_SECTION_CODE, TOKEN_EOF)));

  pstmt->type = STMT_SECTION_CODE;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;
}

enum Err_Grm grammar_line_data(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      tokens_start_with(tokens, 2, TOK_ARR(TOKEN_SECTION_DATA, TOKEN_EOF)));

  pstmt->type = STMT_SECTION_DATA;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;
}

enum Err_Grm grammar_line_label(struct Parsed_Statement *pstmt,
                                const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(
      tokens_start_with(tokens, 2, TOK_ARR(TOKEN_LABEL, TOKEN_EOF)));

  pstmt->type = STMT_LABEL_DEF;
  pstmt->err = PAR_NO_ERROR;

  RETURN_IF_FAIL(token_copy_value(TOK_CURR, pstmt->content.label_def.label_name,
                                  sizeof(pstmt->content.label_def.label_name)),
                 GRM_GENERIC_ERROR);

  return GRM_MATCH;
}

enum Err_Grm grammar_line_identifier(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  NOMATCH_IF_FAIL(token_is(TOK_CURR, TOKEN_IDENTIFIER));
  NOMATCH_IF_FAIL(grammar_identifier_def(pstmt, &TOK_NEXT) == GRM_MATCH);

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;

  RETURN_IF_FAIL(segment_set_dd_total_size(&pstmt->content.data_decl),
                 GRM_GENERIC_ERROR);
  RETURN_IF_FAIL(token_copy_value(TOK_CURR, pstmt->content.data_decl.identifier,
                                  sizeof(pstmt->content.data_decl.identifier)),
                 GRM_GENERIC_ERROR);

  return GRM_MATCH;
}

enum Err_Grm grammar_line_instruction(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[]) {
  struct Instruction_Statement *is = NULL;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  NOMATCH_IF_FAIL(token_is(TOK_CURR, TOKEN_INSTRUCTION));

  NOMATCH_IF_FAIL(grammar_instruction_rhs(pstmt, &TOK_NEXT) == GRM_MATCH);

  pstmt->type = STMT_INSTRUCTION;
  pstmt->err = PAR_NO_ERROR;
  is = &pstmt->content.instruction;
  is->descriptor = instruction_find(TOK_CURR->value, strlen(TOK_CURR->value),
                                    is->operands[0].type, is->operands[1].type);
  RETURN_IF_FAIL(is->descriptor, GRM_GENERIC_ERROR);

  return GRM_MATCH;
}

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  struct Instruction_Statement *is = NULL;
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);
  is = &pstmt->content.instruction;

  if (token_is_eof(TOK_CURR)) {
    RETURN_IF_FAIL(_set_ops_none(is), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (tokens_start_with(tokens, 2, TOK_ARR(TOKEN_LABEL, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_label(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (tokens_start_with(tokens, 2, TOK_ARR(TOKEN_REGISTER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_register(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_number(is, TOK_CURR, 0), GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (tokens_start_with(tokens, 2,
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
  NOMATCH_IF_FAIL(pstmt && tokens && *tokens);

  if (tokens_start_with(tokens, 2, TOK_ARR(TOKEN_REGISTER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_register(&pstmt->content.instruction, TOK_CURR, 1),
                   GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (tokens_start_with(tokens, 2, TOK_ARR(TOKEN_NUMBER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_number(&pstmt->content.instruction, TOK_CURR, 1),
                   GRM_GENERIC_ERROR);
    return GRM_MATCH;
  } else if (tokens_start_with(
                 tokens, 3,
                 TOK_ARR(TOKEN_OFFSET, TOKEN_IDENTIFIER, TOKEN_EOF))) {
    RETURN_IF_FAIL(_set_op_offset(&pstmt->content.instruction, tokens[1], 1),
                   GRM_GENERIC_ERROR);
    return GRM_MATCH;
  }

  return GRM_NO_MATCH;
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
  RETURN_IF_FAIL(token_copy_value(token, is->operands[idx].value.label,
                                  sizeof(is->operands[idx].value.label)),
                 0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
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
      token_copy_value(token, is->operands[idx].value.register_name,
                       sizeof(is->operands[idx].value.register_name)),
      0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_REG;
  is->operands[idx].specifier = OPS_NONE;

  return 1;
}

// set is->op[idx] to number saved in string format in token->value
static int _set_op_number(struct Instruction_Statement *is,
                          const struct Token *token, size_t idx) {
  RETURN_IF_FAIL(
      is && token && idx < sizeof(is->operands) / sizeof(struct Operand), 0);
  RETURN_IF_FAIL(
      token_parse_int32(token, &is->operands[idx].value.immediate_value), 0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_NONE;

  return 1;
}

static int _set_op_offset(struct Instruction_Statement *is,
                          const struct Token *token, size_t idx) {
  RETURN_IF_FAIL(
      is && token && idx < sizeof(is->operands) / sizeof(struct Operand), 0);
  RETURN_IF_FAIL(token_copy_value(token, is->operands[idx].value.label,
                                  sizeof(is->operands[idx].value.label)),
                 0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_OFFSET;

  return 1;
}

static int _set_op_count(struct Instruction_Statement *is, size_t idx) {
  RETURN_IF_FAIL(is, 0);
  RETURN_IF_FAIL(idx < INT_MAX, 0); // idx > sizeof(int)
  if ((int)(idx + 1) > is->operand_count) {
    is->operand_count = (int)(idx + 1);
  }
  return 1;
}
