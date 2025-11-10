#include <inttypes.h>
#include <limits.h>
#include <stdint.h>

#include "common.h"
#include "parser_grammar.h"
#include "parser_instruction.h"
#include "parser_token.h"

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]) {
  struct Instruction_Statement *is = NULL;
  NOMATCH_IF_FAIL_ERR(pstmt && tokens && *tokens,
                      "The arguments were invalid.");
  is = &pstmt->content.instruction;

  if (token_is_eof(TOK_CURR)) { // some instruction have no operand
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

    // some instruction have multiple (two) operands
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
  NOMATCH_IF_FAIL_ERR(pstmt && tokens && *tokens,
                      "The arguments were invalid.");

  // different types of 2nd instruction argument
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

int _set_ops_none(struct Instruction_Statement *is) {
  RET_STDERR_IF_FAIL(is, 0,
                     "Tried set operands to NONE, but the pointer to "
                     "instruction statement is NULL.");

  is->operand_count = 0;
  is->operands[0].type = OP_NONE;
  is->operands[1].type = OP_NONE;

  return 1;
}

int _set_op_label(struct Instruction_Statement *is, const struct Token *token,
                  size_t idx) {
  RET_STDERR_IF_FAIL(is && token &&
                         idx < sizeof(is->operands) / sizeof(struct Operand),
                     0, "Invalid arguments.");
  RETURN_IF_FAIL(token_copy_value(token, is->operands[idx].value.label,
                                  sizeof(is->operands[idx].value.label)),
                 0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_LABEL;

  return 1;
}

int _set_op_register(struct Instruction_Statement *is,
                     const struct Token *token, size_t idx) {
  RET_STDERR_IF_FAIL(is && token &&
                         idx < sizeof(is->operands) / sizeof(struct Operand),
                     0, "Invalid arguments.");
  RETURN_IF_FAIL(
      token_copy_value(token, is->operands[idx].value.register_name,
                       sizeof(is->operands[idx].value.register_name)),
      0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_REG;
  is->operands[idx].specifier = OPS_NONE;

  return 1;
}

int _set_op_number(struct Instruction_Statement *is, const struct Token *token,
                   size_t idx) {
  RET_STDERR_IF_FAIL(is && token &&
                         idx < sizeof(is->operands) / sizeof(struct Operand),
                     0, "Invalid arguments.");
  RETURN_IF_FAIL(
      token_parse_int32(token, &is->operands[idx].value.immediate_value), 0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_NONE;

  return 1;
}

int _set_op_offset(struct Instruction_Statement *is, const struct Token *token,
                   size_t idx) {
  RET_STDERR_IF_FAIL(is && token &&
                         idx < sizeof(is->operands) / sizeof(struct Operand),
                     0, "Invalid arguments.");
  RETURN_IF_FAIL(token_copy_value(token, is->operands[idx].value.label,
                                  sizeof(is->operands[idx].value.label)),
                 0);
  RETURN_IF_FAIL(_set_op_count(is, idx), 0);
  is->operands[idx].type = OP_IMM32;
  is->operands[idx].specifier = OPS_OFFSET;

  return 1;
}

int _set_op_count(struct Instruction_Statement *is, size_t idx) {
  RET_STDERR_IF_FAIL(is, 0, "The given instruction statement is NULL.");
  RETURN_IF_FAIL(idx < INT_MAX, 0); // idx > sizeof(int)
  if ((int)(idx + 1) > is->operand_count) {
    is->operand_count = (int)(idx + 1);
  }
  return 1;
}
