#include "parser_grammar.h"
#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "parser_data.h"
#include <string.h>

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

// TODO:
enum Err_Grm grammar_line_instruction(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_LABEL);

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_def(struct Parsed_Statement *pstmt,
                                    const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  CLEANUP_IF_FAIL(tokens[0]->type == TOKEN_DATA_TYPE);

  if (strcmp(tokens[0]->value, "DWORD") || strcmp(tokens[0]->value, "DW")) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec(pstmt, &tokens[1]));
    pstmt->content.data_decl.type = DATA_DWORD;
  }
  if (strcmp(tokens[0]->value, "BYTE") || strcmp(tokens[0]->value, "DB")) {
    CLEANUP_IF_FAIL(grammar_identifier_db_dec(pstmt, &tokens[1]));
    pstmt->content.data_decl.type = DATA_BYTE;
  }

  pstmt->type = STMT_DATA_DECL;
  pstmt->err = PAR_NO_ERROR;

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_dw_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  if (tokens[0]->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_DUP) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dup(pstmt, &tokens[0]));
  }
  if (tokens[0]->type == TOKEN_NUMBER) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]));
    // HERE
  }

  return GRM_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

enum Err_Grm grammar_identifier_dw_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]);

enum Err_Grm grammar_identifier_dw_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]);

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]);

// Return 1 if the token is EOF.
enum Err_Grm grammar_eof(struct Parsed_Statement *pstmt,
                         const struct Token *tokens[]);
