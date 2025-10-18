#include "../src/lexer.h"
#include "../src/llist.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test basic single-character tokens
static void test_single_character_tokens(void) {
  printf("Testing single-character tokens...\n");

  const char *line = "( , ) ?";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 5); // 4 tokens + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_LPAREN);
  assert(strcmp(token0->value, "(") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_COMMA);
  assert(strcmp(token1->value, ",") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_RPAREN);
  assert(strcmp(token2->value, ")") == 0);

  struct Token *token3 = llist_get(tokens, 3);
  assert(token3->type == TOKEN_QUESTION);
  assert(strcmp(token3->value, "?") == 0);

  struct Token *token4 = llist_get(tokens, 4);
  assert(token4->type == TOKEN_EOF);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test instruction recognition
static void test_instructions(void) {
  printf("Testing instruction recognition...\n");

  const char *line = "MOV ADD HALT JMP";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 5); // 4 instructions + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_INSTRUCTION);
  assert(strcmp(token0->value, "MOV") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_INSTRUCTION);
  assert(strcmp(token1->value, "ADD") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_INSTRUCTION);
  assert(strcmp(token2->value, "HALT") == 0);

  struct Token *token3 = llist_get(tokens, 3);
  assert(token3->type == TOKEN_INSTRUCTION);
  assert(strcmp(token3->value, "JMP") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test register recognition
static void test_registers(void) {
  printf("Testing register recognition...\n");

  const char *line = "A B C D S SP";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 7); // 6 registers + EOF

  for (size_t i = 0; i < 6; i++) {
    struct Token *token = llist_get(tokens, i);
    assert(token->type == TOKEN_REGISTER);
  }

  struct Token *token_sp = llist_get(tokens, 5);
  assert(strcmp(token_sp->value, "SP") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test positive and negative numbers
static void test_numbers(void) {
  printf("Testing number recognition...\n");

  const char *line = "123 -456 0 -1";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 5); // 4 numbers + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_NUMBER);
  assert(strcmp(token0->value, "123") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_NUMBER);
  assert(strcmp(token1->value, "-456") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_NUMBER);
  assert(strcmp(token2->value, "0") == 0);

  struct Token *token3 = llist_get(tokens, 3);
  assert(token3->type == TOKEN_NUMBER);
  assert(strcmp(token3->value, "-1") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test string literals
static void test_strings(void) {
  printf("Testing string literal recognition...\n");

  const char *line = "\"Hello, world!\" \"Test\"";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 3); // 2 strings + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_STRING);
  assert(strcmp(token0->value, "Hello, world!") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_STRING);
  assert(strcmp(token1->value, "Test") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test empty string
static void test_empty_string(void) {
  printf("Testing empty string literal...\n");

  const char *line = "\"\"";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 2); // 1 empty string + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_STRING);
  assert(strcmp(token0->value, "") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test labels
static void test_labels(void) {
  printf("Testing label recognition...\n");

  const char *line = "@start @loop_123 @_end";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 4); // 3 labels + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_LABEL);
  assert(strcmp(token0->value, "@start") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_LABEL);
  assert(strcmp(token1->value, "@loop_123") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_LABEL);
  assert(strcmp(token2->value, "@_end") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test identifiers (variable names)
static void test_identifiers(void) {
  printf("Testing identifier recognition...\n");

  const char *line = "var1 my_variable x";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 4); // 3 identifiers + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_IDENTIFIER);
  assert(strcmp(token0->value, "var1") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_IDENTIFIER);
  assert(strcmp(token1->value, "my_variable") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_IDENTIFIER);
  assert(strcmp(token2->value, "x") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test section markers
static void test_section_markers(void) {
  printf("Testing section markers...\n");

  const char *line = ".DATA .CODE .KMA";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 4); // 3 sections + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_SECTION_DATA);
  assert(strcmp(token0->value, ".DATA") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_SECTION_CODE);
  assert(strcmp(token1->value, ".CODE") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_KMA);
  assert(strcmp(token2->value, ".KMA") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test data type keywords
static void test_data_types(void) {
  printf("Testing data type keywords...\n");

  const char *line = "DWORD DW BYTE DB";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 5); // 4 data types + EOF

  for (size_t i = 0; i < 4; i++) {
    struct Token *token = llist_get(tokens, i);
    assert(token->type == TOKEN_DATA_TYPE);
  }

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test special keywords
static void test_special_keywords(void) {
  printf("Testing special keywords...\n");

  const char *line = "OFFSET DUP";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 3); // 2 keywords + EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_OFFSET);
  assert(strcmp(token0->value, "OFFSET") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_DUP);
  assert(strcmp(token1->value, "DUP") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test a complete realistic line of assembly
static void test_realistic_instruction(void) {
  printf("Testing realistic instruction line...\n");

  const char *line = "MOV A, 5";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 5); // MOV, A, comma, 5, EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_INSTRUCTION);
  assert(strcmp(token0->value, "MOV") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_REGISTER);
  assert(strcmp(token1->value, "A") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_COMMA);

  struct Token *token3 = llist_get(tokens, 3);
  assert(token3->type == TOKEN_NUMBER);
  assert(strcmp(token3->value, "5") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test data declaration line
static void test_data_declaration(void) {
  printf("Testing data declaration...\n");

  const char *line = "var1 DWORD ?";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 4); // var1, DWORD, ?, EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_IDENTIFIER);
  assert(strcmp(token0->value, "var1") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_DATA_TYPE);
  assert(strcmp(token1->value, "DWORD") == 0);

  struct Token *token2 = llist_get(tokens, 2);
  assert(token2->type == TOKEN_QUESTION);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test comment handling
static void test_comments(void) {
  printf("Testing comment handling...\n");

  const char *line = "MOV A, 5 ; this is a comment";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  // Should only tokenize what's before the semicolon
  assert(tokens->count == 5); // MOV, A, comma, 5, EOF

  struct Token *last_real_token = llist_get(tokens, 3);
  assert(last_real_token->type == TOKEN_NUMBER);
  assert(strcmp(last_real_token->value, "5") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test empty line
static void test_empty_line(void) {
  printf("Testing empty line...\n");

  const char *line = "  ";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 1); // Just EOF

  struct Token *token = llist_get(tokens, 0);
  assert(token->type == TOKEN_EOF);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test line with only a comment
static void test_comment_only_line(void) {
  printf("Testing comment-only line...\n");

  const char *line = "; this is just a comment";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 1); // Just EOF

  struct Token *token = llist_get(tokens, 0);
  assert(token->type == TOKEN_EOF);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test DUP syntax
static void test_dup_syntax(void) {
  printf("Testing DUP syntax...\n");

  const char *line = "arr DWORD 100 DUP(?)";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 8); // arr, DWORD, 100, DUP, (, ?, ), EOF

  struct Token *token3 = llist_get(tokens, 3);
  assert(token3->type == TOKEN_DUP);
  assert(strcmp(token3->value, "DUP") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test OFFSET keyword
static void test_offset_usage(void) {
  printf("Testing OFFSET usage...\n");

  const char *line = "MOV A, OFFSET var1";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 6); // MOV, A, comma, OFFSET, var1, EOF

  struct Token *token3 = llist_get(tokens, 3);
  assert(token3->type == TOKEN_OFFSET);
  assert(strcmp(token3->value, "OFFSET") == 0);

  struct Token *token4 = llist_get(tokens, 4);
  assert(token4->type == TOKEN_IDENTIFIER);
  assert(strcmp(token4->value, "var1") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test that instruction prefixes don't get confused
static void test_instruction_prefixes(void) {
  printf("Testing instruction prefix disambiguation...\n");

  // ADDR should be an identifier, not ADD instruction
  const char *line = "ADD ADDR";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 3); // ADD, ADDR, EOF

  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_INSTRUCTION);
  assert(strcmp(token0->value, "ADD") == 0);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_IDENTIFIER);
  assert(strcmp(token1->value, "ADDR") == 0);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

// Test whitespace handling variations
static void test_various_whitespace(void) {
  printf("Testing various whitespace patterns...\n");

  const char *line = "  MOV   A,5  ";
  struct Llist *tokens = lexer_tokenize_line(line, 1);

  assert(tokens != NULL);
  assert(tokens->count == 5); // MOV, A, comma, 5, EOF

  // Verify tokens are correct despite weird whitespace
  struct Token *token0 = llist_get(tokens, 0);
  assert(token0->type == TOKEN_INSTRUCTION);

  struct Token *token1 = llist_get(tokens, 1);
  assert(token1->type == TOKEN_REGISTER);

  llist_free(tokens, (llist_free_node_data)lexer_free_token);
  printf("  PASSED\n");
}

int main(void) {
  printf("\n=== Running Lexer Unit Tests ===\n\n");

  test_single_character_tokens();
  test_instructions();
  test_registers();
  test_numbers();
  test_strings();
  test_empty_string();
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
