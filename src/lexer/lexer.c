#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "lexer.h"
#include "memory.h"

#include "set_token.h"
#include "token_array.h"

// ===== PRIVATE FUNCTION DECLARATIONS  =====

// Skip all whitespaces or comments in line by INCREMENTING the pos value.
// Return 1 if there is a token waiting to be parsed on pos.
// Return 0 if end of line was reached.
static int _lexer_skip_to_next_token(const char *line, const size_t len,
                                     size_t *pos);

// ===== PUBLIC FUNCTIONS =====

struct Token *lexer_tokenize_line(const char *line, const size_t nl) {
  struct Token_Arr arr = {0};
  struct Token *token = NULL;
  size_t pos = 0;
  size_t len = 0;

  if (!line) {
    return NULL;
  }

  CLEANUP_IF_FAIL(tok_arr_init(&arr));

  len = strlen(line);

  // Process the whole line
  while (_lexer_skip_to_next_token(line, len, &pos)) {
    CLEANUP_IF_FAIL(tok_arr_ensure_capacity(&arr, 1));
    token = &arr.tokens[arr.count];

    CLEANUP_IF_FAIL(set_next_token(token, line, len, &pos, nl));
    arr.count++;

    token = NULL;
  }

  // Add EOF to the end
  CLEANUP_IF_FAIL(tok_arr_ensure_capacity(&arr, 1));
  token = &arr.tokens[arr.count];

  CLEANUP_IF_FAIL(set_token(token, TOKEN_EOF, NULL, nl));
  arr.count++;

  return arr.tokens;

cleanup:
  tok_arr_deinit(&arr);
  return NULL;
}

void lexer_free_tokens(struct Token *tokens) {
  if (tokens) {
    jree(tokens);
  }
  return;
}

const char *token_type_to_str(enum Token_Type type) {
  switch (type) {
  case TOKEN_INSTRUCTION:
    return "INSTRUCTION";
  case TOKEN_REGISTER:
    return "REGISTER";
  case TOKEN_NUMBER:
    return "NUMBER";
  case TOKEN_IDENTIFIER:
    return "IDENTIFIER";
  case TOKEN_LABEL:
    return "LABEL";
  case TOKEN_COMMA:
    return "COMMA";
  case TOKEN_SECTION_DATA:
    return "SECTION_DATA";
  case TOKEN_SECTION_CODE:
    return "SECTION_CODE";
  case TOKEN_KMA:
    return "KMA";
  case TOKEN_OFFSET:
    return "OFFSET";
  case TOKEN_QUESTION:
    return "QUESTION";
  case TOKEN_STRING:
    return "STRING";
  case TOKEN_DATA_TYPE:
    return "DATA_TYPE";
  case TOKEN_DUP:
    return "DUP";
  case TOKEN_LPAREN:
    return "LPAREN";
  case TOKEN_RPAREN:
    return "RPAREN";
  case TOKEN_EOF:
    return "EOF";
  case TOKEN_UNKNOWN:
  default:
    return "UNKNOWN";
  }
}

void print_token(const struct Token *token) {
  if (!token) {
    printf("(null token)\n");
    return;
  }

  printf("Token{ type=%s, value=\"%s\", line=%zu }\n",
         token_type_to_str(token->type), token->value, token->line_number);
}

void print_tokens(const struct Token *tokens) {
  if (!tokens) {
    printf("(null token array)\n");
    return;
  }

  size_t i = 0;
  while (tokens[i].type != TOKEN_EOF) {
    printf("[%zu] ", i);
    print_token(&tokens[i]);
    i++;
  }
  printf("[%zu] Token{ type=EOF }\n", i);
}

// ===== PRIVATE FUNCTIONS =====

static int _lexer_skip_to_next_token(const char *line, const size_t len,
                                     size_t *pos) {
  if (!line || !pos) {
    return 0;
  }

  if (*pos >= len) {
    *pos = len;
    return 0;
  }

  while (*pos < len && isspace((unsigned char)line[*pos])) { // skip whitespaces
    (*pos)++;
  }

  if (*pos >= len) { // reached EO Line
    *pos = len;
    return 0;
  }

  if (line[*pos] == ';') { // reached comments
    return 0;
  }
  return 1; // found something meaningful
}
