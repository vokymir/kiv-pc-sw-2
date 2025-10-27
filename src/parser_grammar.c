#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "parser_data.h"
#include "parser_grammar.h"

// Check wheter next <count> tokens are not EOF.
// On success return 1, on failure 0.
static int _exist_tokens(const struct Token *tokens[], size_t count);

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
  segment = &pstmt->content.data_decl.segments[segment_idx];

  if (token->type == TOKEN_NUMBER && tokens[1]->type == TOKEN_DUP) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dup(pstmt, &tokens[0]) == GRM_MATCH);
    segment->type = INIT_SEG_DUP;
  } else if (token->type == TOKEN_NUMBER) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]) == GRM_MATCH);
    segment->data.value = strtoimax(token->value, &end, 10);

    CLEANUP_IF_FAIL(token->value != end);
    segment->type = INIT_SEG_VALUE;
    segment->is_uninit = 0;
    pstmt->content.data_decl.is_fully_uninit = 0;
    segment->element_count = 1;
  } else if (token->type == TOKEN_QUESTION) {
    CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[1]) == GRM_MATCH);

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
    CLEANUP_IF_FAIL(pstmt); // TODO: Adept for pstmt init or smth.
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
  segment = &pstmt->content.data_decl.segments[segment_idx];

  // parse next part of line
  CLEANUP_IF_FAIL(grammar_identifier_dw_dec2(pstmt, &tokens[dup_len]) ==
                  GRM_MATCH);

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
enum Err_Grm grammar_eof(const struct Parsed_Statement *pstmt,
                         const struct Token *tokens[]) {

  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);
  return (tokens[0]->type == TOKEN_EOF) ? GRM_MATCH : GRM_NO_MATCH;

cleanup:
  return GRM_NO_MATCH;
}

static int _exist_tokens(const struct Token *tokens[], size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL(*tokens);

  for (i = 0; i < count; i++) {
    CLEANUP_IF_FAIL(tokens[i]->type != TOKEN_EOF);
  }

  return 1;

cleanup:
  return 0;
}
