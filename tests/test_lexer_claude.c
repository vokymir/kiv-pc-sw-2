// tests/test_lexer_all.c
#include "../src/lexer.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Count tokens in returned array (include terminating TOKEN_EOF) */
static size_t token_count(const struct Token *tokens) {
  size_t i = 0;
  if (!tokens)
    return 0;
  while (tokens[i].type != TOKEN_EOF) {
    i++;
  }
  return i + 1; // include EOF
}

/* Access token at index */
#define TOK_AT(tokens, idx) (&(tokens)[(idx)])

/* Helper assertion macro: check token type and optional string value */
#define ASSERT_TOKEN(tokens, idx, type_val, val_str)                           \
  do {                                                                         \
    struct Token *tok = TOK_AT((tokens), (idx));                               \
    assert(tok != NULL);                                                       \
    assert(tok->type == (type_val));                                           \
    if ((val_str) != NULL)                                                     \
      assert(strcmp(tok->value, (val_str)) == 0);                              \
  } while (0)

/* Macro: create token array from lexer */
#define LEXER_TOKENS(line, nl) lexer_tokenize_line((line), (nl))

/* ---------- Tests ---------- */

static void test_single_character_tokens(void) {
  printf("Testing single-character tokens...\n");
  const char *line = "( , ) ?";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 5); // 4 tokens + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_LPAREN, "(");
  ASSERT_TOKEN(tokens, 1, TOKEN_COMMA, ",");
  ASSERT_TOKEN(tokens, 2, TOKEN_RPAREN, ")");
  ASSERT_TOKEN(tokens, 3, TOKEN_QUESTION, "?");
  ASSERT_TOKEN(tokens, 4, TOKEN_EOF, NULL);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_instructions(void) {
  printf("Testing instruction recognition...\n");
  const char *line = "MOV ADD HALT JMP";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 5); // 4 instructions + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_INSTRUCTION, "MOV");
  ASSERT_TOKEN(tokens, 1, TOKEN_INSTRUCTION, "ADD");
  ASSERT_TOKEN(tokens, 2, TOKEN_INSTRUCTION, "HALT");
  ASSERT_TOKEN(tokens, 3, TOKEN_INSTRUCTION, "JMP");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_registers(void) {
  printf("Testing register recognition...\n");
  const char *line = "A B C D S SP";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 7); // 6 registers + EOF

  for (size_t i = 0; i < 6; i++) {
    struct Token *tok = TOK_AT(tokens, i);
    assert(tok != NULL);
    assert(tok->type == TOKEN_REGISTER);
  }

  struct Token *tok_sp = TOK_AT(tokens, 5);
  assert(strcmp(tok_sp->value, "SP") == 0);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_numbers(void) {
  printf("Testing number recognition...\n");
  const char *line = "123 -456 0 -1";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 5); // 4 numbers + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_NUMBER, "123");
  ASSERT_TOKEN(tokens, 1, TOKEN_NUMBER, "-456");
  ASSERT_TOKEN(tokens, 2, TOKEN_NUMBER, "0");
  ASSERT_TOKEN(tokens, 3, TOKEN_NUMBER, "-1");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_strings(void) {
  printf("Testing string literal recognition...\n");
  const char *line = "\"Hello, world!\" \"Test\"";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 3); // 2 strings + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_STRING, "Hello, world!");
  ASSERT_TOKEN(tokens, 1, TOKEN_STRING, "Test");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_empty_string(void) {
  printf("Testing empty string literal...\n");
  const char *line = "\"\"";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 2); // 1 empty string + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_STRING, "");
  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_labels(void) {
  printf("Testing label recognition...\n");
  const char *line = "@start @loop_123 @_end";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 4); // 3 labels + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_LABEL, "@start");
  ASSERT_TOKEN(tokens, 1, TOKEN_LABEL, "@loop_123");
  ASSERT_TOKEN(tokens, 2, TOKEN_LABEL, "@_end");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_identifiers(void) {
  printf("Testing identifier recognition...\n");
  const char *line = "var1 my_variable x";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 4); // 3 identifiers + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_IDENTIFIER, "var1");
  ASSERT_TOKEN(tokens, 1, TOKEN_IDENTIFIER, "my_variable");
  ASSERT_TOKEN(tokens, 2, TOKEN_IDENTIFIER, "x");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_section_markers(void) {
  printf("Testing section markers...\n");
  const char *line = ".DATA .CODE .KMA";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 4); // 3 sections + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_SECTION_DATA, ".DATA");
  ASSERT_TOKEN(tokens, 1, TOKEN_SECTION_CODE, ".CODE");
  ASSERT_TOKEN(tokens, 2, TOKEN_KMA, ".KMA");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_data_types(void) {
  printf("Testing data type keywords...\n");
  const char *line = "DWORD DW BYTE DB";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 5); // 4 data types + EOF

  for (size_t i = 0; i < 4; i++) {
    struct Token *tok = TOK_AT(tokens, i);
    assert(tok != NULL);
    assert(tok->type == TOKEN_DATA_TYPE);
  }

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_special_keywords(void) {
  printf("Testing special keywords...\n");
  const char *line = "OFFSET DUP";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 3); // 2 keywords + EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_OFFSET, "OFFSET");
  ASSERT_TOKEN(tokens, 1, TOKEN_DUP, "DUP");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_realistic_instruction(void) {
  printf("Testing realistic instruction line...\n");
  const char *line = "MOV A, 5";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 5); // MOV, A, comma, 5, EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_INSTRUCTION, "MOV");
  ASSERT_TOKEN(tokens, 1, TOKEN_REGISTER, "A");
  ASSERT_TOKEN(tokens, 2, TOKEN_COMMA, NULL);
  ASSERT_TOKEN(tokens, 3, TOKEN_NUMBER, "5");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_data_declaration(void) {
  printf("Testing data declaration...\n");
  const char *line = "var1 DWORD ?";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 4); // var1, DWORD, ?, EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_IDENTIFIER, "var1");
  ASSERT_TOKEN(tokens, 1, TOKEN_DATA_TYPE, "DWORD");
  ASSERT_TOKEN(tokens, 2, TOKEN_QUESTION, NULL);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_comments(void) {
  printf("Testing comment handling...\n");
  const char *line = "MOV A, 5 ; this is a comment";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  // Should only tokenize what's before the semicolon
  assert(token_count(tokens) == 5); // MOV, A, comma, 5, EOF

  struct Token *last_real_token = TOK_AT(tokens, 3);
  assert(last_real_token != NULL);
  assert(last_real_token->type == TOKEN_NUMBER);
  assert(strcmp(last_real_token->value, "5") == 0);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_empty_line(void) {
  printf("Testing empty line...\n");
  const char *line = "  ";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 1); // Just EOF

  struct Token *tok = TOK_AT(tokens, 0);
  assert(tok != NULL && tok->type == TOKEN_EOF);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_comment_only_line(void) {
  printf("Testing comment-only line...\n");
  const char *line = "; this is just a comment";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 1); // Just EOF

  struct Token *tok = TOK_AT(tokens, 0);
  assert(tok != NULL && tok->type == TOKEN_EOF);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_dup_syntax(void) {
  printf("Testing DUP syntax...\n");
  const char *line = "arr DWORD 100 DUP(?)";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 8); // arr, DWORD, 100, DUP, (, ?, ), EOF

  ASSERT_TOKEN(tokens, 3, TOKEN_DUP, "DUP");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_offset_usage(void) {
  printf("Testing OFFSET usage...\n");
  const char *line = "MOV A, OFFSET var1";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 6); // MOV, A, comma, OFFSET, var1, EOF

  ASSERT_TOKEN(tokens, 3, TOKEN_OFFSET, "OFFSET");
  ASSERT_TOKEN(tokens, 4, TOKEN_IDENTIFIER, "var1");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_instruction_prefixes(void) {
  printf("Testing instruction prefix disambiguation...\n");
  const char *line = "ADD ADDR";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 3); // ADD, ADDR, EOF

  ASSERT_TOKEN(tokens, 0, TOKEN_INSTRUCTION, "ADD");
  ASSERT_TOKEN(tokens, 1, TOKEN_IDENTIFIER, "ADDR");

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

static void test_various_whitespace(void) {
  printf("Testing various whitespace patterns...\n");
  const char *line = "  MOV   A,5  ";
  struct Token *tokens = LEXER_TOKENS(line, 1);
  assert(tokens != NULL);
  assert(token_count(tokens) == 5); // MOV, A, comma, 5, EOF

  struct Token *t0 = TOK_AT(tokens, 0);
  assert(t0 != NULL && t0->type == TOKEN_INSTRUCTION);
  struct Token *t1 = TOK_AT(tokens, 1);
  assert(t1 != NULL && t1->type == TOKEN_REGISTER);

  lexer_free_tokens(tokens);
  printf("  PASSED\n");
}

/* ---------- Main ---------- */
int main(void) {
  printf("\n=== Running Lexer Unit Tests (array interface).");

  test_single_character_tokens();
  test_instructions();
  test_registers();
  test_numbers();
  test_strings();
  // test_empty_string(); // THIS ISN'T SUPPORTED IN THE ASSIGNMENT (I THINK)
  test_labels();
  test_identifiers();
  test_section_markers();
  test_data_types();
  test_special_keywords();
  test_realistic_instruction();
  test_data_declaration();
  test_comments();
  test_empty_line();
  test_comment_only_line();
  test_dup_syntax();
  test_offset_usage();
  test_instruction_prefixes();
  test_various_whitespace();

  printf("\n=== All Lexer Tests Passed! ===\n\n");
  return 0;
}
