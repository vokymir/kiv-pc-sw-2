#include <inttypes.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "instruction.h"
#include "lexer.h"
#include "parser.h"

#include "parser_grammar.h"
#include "parser_identifier.h"
#include "parser_instruction.h"
#include "parser_segment.h"
#include "parser_token.h"

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

  // recursively look for all declaration segments
  NOMATCH_IF_FAIL(grammar_identifier_def(pstmt, &TOK_NEXT) == GRM_MATCH);

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;

  // count all segment counts and add them to total size
  RETURN_IF_FAIL(segment_set_dd_total_size(&pstmt->content.data_decl),
                 GRM_GENERIC_ERROR);
  // copy identifier
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
