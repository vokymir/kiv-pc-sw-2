#include "../src/datastruc.h"
#include "../src/lexer.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

const char *lexer_token_type_to_str(enum TokenType type);

// Helper to print tokens (optional)
static void print_token(void *data) {
  struct Token *t = (struct Token *)data;
  printf("[%s:%d] \"%s\"\n", lexer_token_type_to_str(t->type),
         (int)t->line_number, t->value);
}

// Convert enum TokenType to string (for debug output)
const char *lexer_token_type_to_str(enum TokenType type) {
  switch (type) {
  case TOKEN_SECTION_DATA:
    return "SECTION_DATA";
  case TOKEN_SECTION_CODE:
    return "SECTION_CODE";
  case TOKEN_REGISTER:
    return "REGISTER";
  case TOKEN_INSTRUCTION:
    return "INSTRUCTION";
  case TOKEN_LABEL:
    return "LABEL";
  case TOKEN_NUMBER:
    return "NUMBER";
  case TOKEN_STRING:
    return "STRING";
  case TOKEN_COMMA:
    return "COMMA";
  case TOKEN_LPAREN:
    return "LPAREN";
  case TOKEN_RPAREN:
    return "RPAREN";
  case TOKEN_IDENTIFIER:
    return "IDENTIFIER";
  case TOKEN_EOF:
    return "EOF";
  case TOKEN_KMA:
    return "KMA";
  case TOKEN_OFFSET:
    return "OFFSET";
  case TOKEN_QUESTION:
    return "?";
  case TOKEN_DATA_TYPE:
    return "DATA TYPE";
  case TOKEN_DUP:
    return "DUP";
  case TOKEN_UNKNOWN:
    return "UNKNOWN";
  default:
    return "ERROR";
  }
}

int main(void) {
  printf("Running lexer tests...\n");

  const char *line = "MOV A, 42 ; comment\n";
  size_t nl = 1;

  // === Tokenize ===
  struct DS_Llist *tokens = lexer_tokenize_line(line, nl);
  assert(tokens != NULL);
  assert(tokens->count > 0);
  printf("Tokenized successfully, token count = %zu\n", tokens->count);

  // === Inspect tokens ===
  struct Token *t0 = (struct Token *)ds_llist_get_data(tokens, 0);
  struct Token *t1 = (struct Token *)ds_llist_get_data(tokens, 1);
  struct Token *t2 = (struct Token *)ds_llist_get_data(tokens, 2);

  assert(t0 && t1 && t2);
  assert(t0->type == TOKEN_INSTRUCTION);
  assert(strcmp(t0->value, "MOV") == 0);

  assert(t1->type == TOKEN_REGISTER);
  assert(strcmp(t1->value, "A") == 0);

  assert(t2->type == TOKEN_COMMA);

  printf("First 3 tokens verified (MOV, A, ,)\n");

  // === Last token should be EOF ===
  struct Token *last =
      (struct Token *)ds_llist_get_data(tokens, tokens->count - 1);
  assert(last && last->type == TOKEN_EOF);
  printf("EOF token present.\n");

  // === Optional print ===
  printf("Token list:\n");
  ds_llist_foreach(tokens, print_token);

  // === Free ===
  ds_llist_free_node_data func = (ds_llist_free_node_data)lexer_free_token;
  ds_llist_free(tokens, func);
  assert(jemory() == 0);
  printf("Freed tokens, no leaks detected.\n");

  printf("✅ All lexer tests passed!\n");
  return 0;
}
