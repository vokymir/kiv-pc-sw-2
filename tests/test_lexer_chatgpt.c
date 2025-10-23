#include "../src/lexer.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Forward decl
const char *lexer_token_type_to_str(enum Token_Type type);

// Helper to print a single token (optional)
static void print_token(const struct Token *t) {
  printf("[%s:%zu] \"%s\"\n", lexer_token_type_to_str(t->type), t->line_number,
         t->value);
}

// Convert enum TokenType to string (for debug output)
const char *lexer_token_type_to_str(enum Token_Type type) {
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
    return "DATA_TYPE";
  case TOKEN_DUP:
    return "DUP";
  case TOKEN_UNKNOWN:
    return "UNKNOWN";
  default:
    return "ERROR";
  }
}

// Count tokens in the returned array (includes the terminating TOKEN_EOF)
static size_t token_count(const struct Token *tokens) {
  size_t i = 0;
  if (!tokens)
    return 0;
  while (tokens[i].type != TOKEN_EOF) {
    i++;
  }
  // include the EOF token in the count to match previous behavior
  return i + 1;
}

int main(void) {
  printf("Running lexer tests (array-based interface)...\n");

  const char *line = "MOV A, 42 ; comment\n";
  size_t nl = 1;

  // === Tokenize line ===
  struct Token *tokens = lexer_tokenize_line(line, nl);
  assert(tokens != NULL);

  size_t count = token_count(tokens);
  assert(count > 0);
  printf("Tokenized successfully, token count = %zu\n", count);

  // === Inspect tokens ===
  struct Token *t0 = &tokens[0];
  struct Token *t1 = &tokens[1];
  struct Token *t2 = &tokens[2];

  assert(t0 && t1 && t2);
  assert(t0->type == TOKEN_INSTRUCTION);
  assert(strcmp(t0->value, "MOV") == 0);

  assert(t1->type == TOKEN_REGISTER);
  assert(strcmp(t1->value, "A") == 0);

  assert(t2->type == TOKEN_COMMA);
  printf("First 3 tokens verified (MOV, A, ,)\n");

  // === Last token should be EOF ===
  struct Token *last = &tokens[count - 1];
  assert(last && last->type == TOKEN_EOF);
  printf("EOF token present.\n");

  // === Optional print ===
  printf("Token list:\n");
  for (size_t i = 0; i < count; ++i) {
    print_token(&tokens[i]);
  }

  // === Free ===
  lexer_free_tokens(tokens);
  assert(jemory() == 0);
  printf("Freed tokens, no leaks detected.\n");

  printf("✅ All lexer tests passed!\n");
  return 0;
}
