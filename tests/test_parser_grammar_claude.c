#include "../src/lexer.h"
#include "../src/memory.h"
#include "../src/parser.h"
#include "../src/parser_grammar.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to create a token for testing
struct Token *create_test_token(enum Token_Type type, const char *value,
                                size_t line) {
  struct Token *tok = jalloc(sizeof(struct Token));
  tok->type = type;
  strncpy(tok->value, value, TOKEN_MAX_VALUE_LEN - 1);
  tok->value[TOKEN_MAX_VALUE_LEN - 1] = '\0';
  tok->line_number = line;
  return tok;
}

// Helper to free token array
void cleanup_tokens(struct Token **tokens, size_t count) {
  for (size_t i = 0; i < count; i++) {
    jree(tokens[i]);
  }
}

// Test: grammar_line should recognize EOF as empty line
void test_grammar_line_empty() {
  printf("Running test_grammar_line_empty...\n");

  struct Token *tokens[1];
  tokens[0] = create_test_token(TOKEN_EOF, "", 1);

  const struct Token *const_tokens[1] = {tokens[0]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_NONE, 1);

  enum Err_Grm result = grammar_line(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_NONE);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 1);

  printf("  PASSED\n");
}

// Test: grammar_line_kma should match KMA directive
void test_grammar_line_kma_match() {
  printf("Running test_grammar_line_kma_match...\n");

  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_KMA, ".KMA", 1);
  tokens[1] = create_test_token(TOKEN_EOF, "", 1);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_NONE, 1);

  enum Err_Grm result = grammar_line_kma(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_KMA);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_kma should not match non-KMA tokens
void test_grammar_line_kma_no_match() {
  printf("Running test_grammar_line_kma_no_match...\n");

  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_SECTION_DATA, ".DATA", 1);
  tokens[1] = create_test_token(TOKEN_EOF, "", 1);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  enum Statement_Type original_type = STMT_NONE;
  p_stmt_init(&stmt, original_type, 1);

  enum Err_Grm result = grammar_line_kma(&stmt, const_tokens);

  // Should not match and statement should be unchanged
  assert(result == GRM_NO_MATCH);
  assert(stmt.type == original_type);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_code should match .CODE directive
void test_grammar_line_code_match() {
  printf("Running test_grammar_line_code_match...\n");

  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_SECTION_CODE, ".CODE", 5);
  tokens[1] = create_test_token(TOKEN_EOF, "", 5);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_NONE, 5);

  enum Err_Grm result = grammar_line_code(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_SECTION_CODE);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_code should not match non-CODE tokens
void test_grammar_line_code_no_match() {
  printf("Running test_grammar_line_code_no_match...\n");

  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_KMA, ".KMA", 5);
  tokens[1] = create_test_token(TOKEN_EOF, "", 5);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_NONE, 5);

  enum Err_Grm result = grammar_line_code(&stmt, const_tokens);

  assert(result == GRM_NO_MATCH);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_data should match .DATA directive
void test_grammar_line_data_match() {
  printf("Running test_grammar_line_data_match...\n");

  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_SECTION_DATA, ".DATA", 10);
  tokens[1] = create_test_token(TOKEN_EOF, "", 10);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_NONE, 10);

  enum Err_Grm result = grammar_line_data(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_SECTION_DATA);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_label should match label definition
void test_grammar_line_label_match() {
  printf("Running test_grammar_line_label_match...\n");

  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_LABEL, "@start", 15);
  tokens[1] = create_test_token(TOKEN_EOF, "", 15);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_LABEL_DEF, 15);

  enum Err_Grm result = grammar_line_label(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_LABEL_DEF);
  assert(strcmp(stmt.content.label_def.label_name, "@start") == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_label should handle different label names
void test_grammar_line_label_various_names() {
  printf("Running test_grammar_line_label_various_names...\n");

  const char *label_names[] = {"@loop", "@end", "@main", "@func_1"};

  for (int i = 0; i < 4; i++) {
    struct Token *tokens[2];
    tokens[0] = create_test_token(TOKEN_LABEL, label_names[i], 20);
    tokens[1] = create_test_token(TOKEN_EOF, "", 20);

    const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

    struct Parsed_Statement stmt;
    p_stmt_init(&stmt, STMT_LABEL_DEF, 20);

    enum Err_Grm result = grammar_line_label(&stmt, const_tokens);

    assert(result == GRM_MATCH);
    assert(strcmp(stmt.content.label_def.label_name, label_names[i]) == 0);

    p_stmt_deinit(&stmt);
    cleanup_tokens(tokens, 2);
  }

  printf("  PASSED\n");
}

// Test: grammar_line_identifier should match data declaration with DWORD
void test_grammar_line_identifier_dword() {
  printf("Running test_grammar_line_identifier_dword...\n");

  // var1 DWORD 42
  struct Token *tokens[4];
  tokens[0] = create_test_token(TOKEN_IDENTIFIER, "var1", 25);
  tokens[1] = create_test_token(TOKEN_DATA_TYPE, "DWORD", 25);
  tokens[2] = create_test_token(TOKEN_NUMBER, "42", 25);
  tokens[3] = create_test_token(TOKEN_EOF, "", 25);

  const struct Token *const_tokens[4] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 25);

  enum Err_Grm result = grammar_line_identifier(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_DATA_DECL);
  assert(strcmp(stmt.content.data_decl.identifier, "var1") == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 4);

  printf("  PASSED\n");
}

// Test: grammar_line_identifier should match data declaration with BYTE
void test_grammar_line_identifier_byte() {
  printf("Running test_grammar_line_identifier_byte...\n");

  // char1 BYTE 65
  struct Token *tokens[4];
  tokens[0] = create_test_token(TOKEN_IDENTIFIER, "char1", 30);
  tokens[1] = create_test_token(TOKEN_DATA_TYPE, "BYTE", 30);
  tokens[2] = create_test_token(TOKEN_NUMBER, "65", 30);
  tokens[3] = create_test_token(TOKEN_EOF, "", 30);

  const struct Token *const_tokens[4] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 30);

  enum Err_Grm result = grammar_line_identifier(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_DATA_DECL);
  assert(stmt.content.data_decl.type == DATA_BYTE);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 4);

  printf("  PASSED\n");
}

// Test: grammar_identifier_dw_dec should handle single value
void test_grammar_identifier_dw_dec_single() {
  printf("Running test_grammar_identifier_dw_dec_single...\n");

  // Just the "42" part
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_NUMBER, "42", 35);
  tokens[1] = create_test_token(TOKEN_EOF, "", 35);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 35);
  stmt.content.data_decl.type = DATA_DWORD;

  enum Err_Grm result = grammar_identifier_dw_dec(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.data_decl.segment_count >= 1);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_identifier_dw_dec should handle uninitialized value
void test_grammar_identifier_dw_dec_uninit() {
  printf("Running test_grammar_identifier_dw_dec_uninit...\n");

  // Just the "?" part
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_QUESTION, "?", 40);
  tokens[1] = create_test_token(TOKEN_EOF, "", 40);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 40);
  stmt.content.data_decl.type = DATA_DWORD;

  enum Err_Grm result = grammar_identifier_dw_dec(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.data_decl.segments[0].is_uninit == 1);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_identifier_dw_dup should handle DUP with value
void test_grammar_identifier_dw_dup_with_value() {
  printf("Running test_grammar_identifier_dw_dup_with_value...\n");

  // 10 DUP(5)
  struct Token *tokens[6];
  tokens[0] = create_test_token(TOKEN_NUMBER, "10", 45);
  tokens[1] = create_test_token(TOKEN_DUP, "DUP", 45);
  tokens[2] = create_test_token(TOKEN_LPAREN, "(", 45);
  tokens[3] = create_test_token(TOKEN_NUMBER, "5", 45);
  tokens[4] = create_test_token(TOKEN_RPAREN, ")", 45);
  tokens[5] = create_test_token(TOKEN_EOF, "", 45);

  const struct Token *const_tokens[6] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4], tokens[5]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 45);
  stmt.content.data_decl.type = DATA_DWORD;
  stmt.content.data_decl.segments = calloc(1, sizeof(struct Init_Segment));
  stmt.content.data_decl.segment_count = 1;

  enum Err_Grm result = grammar_identifier_dw_dup(&stmt, const_tokens, 0);

  assert(result == GRM_MATCH);
  assert(stmt.content.data_decl.segments[0].type == INIT_SEG_DUP);
  assert(stmt.content.data_decl.segments[0].data.dup.count == 10);
  assert(stmt.content.data_decl.segments[0].data.dup.value == 5);
  assert(stmt.content.data_decl.segments[0].is_uninit == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 6);

  printf("  PASSED\n");
}

// Test: grammar_identifier_dw_dup should handle DUP with uninitialized
void test_grammar_identifier_dw_dup_uninit() {
  printf("Running test_grammar_identifier_dw_dup_uninit...\n");

  // 20 DUP(?)
  struct Token *tokens[6];
  tokens[0] = create_test_token(TOKEN_NUMBER, "20", 50);
  tokens[1] = create_test_token(TOKEN_DUP, "DUP", 50);
  tokens[2] = create_test_token(TOKEN_LPAREN, "(", 50);
  tokens[3] = create_test_token(TOKEN_QUESTION, "?", 50);
  tokens[4] = create_test_token(TOKEN_RPAREN, ")", 50);
  tokens[5] = create_test_token(TOKEN_EOF, "", 50);

  const struct Token *const_tokens[6] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4], tokens[5]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 50);
  stmt.content.data_decl.type = DATA_DWORD;
  stmt.content.data_decl.segments = calloc(1, sizeof(struct Init_Segment));
  stmt.content.data_decl.segment_count = 1;

  enum Err_Grm result = grammar_identifier_dw_dup(&stmt, const_tokens, 0);

  assert(result == GRM_MATCH);
  assert(stmt.content.data_decl.segments[0].type == INIT_SEG_DUP);
  assert(stmt.content.data_decl.segments[0].data.dup.count == 20);
  assert(stmt.content.data_decl.segments[0].is_uninit == 1);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 6);

  printf("  PASSED\n");
}

// Test: grammar_identifier_db_dec should handle string
void test_grammar_identifier_db_dec_string() {
  printf("Running test_grammar_identifier_db_dec_string...\n");

  // "Hello"
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_STRING, "Hello", 55);
  tokens[1] = create_test_token(TOKEN_EOF, "", 55);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 55);
  stmt.content.data_decl.type = DATA_BYTE;

  enum Err_Grm result = grammar_identifier_db_dec(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.data_decl.segments[0].type == INIT_SEG_STRING);
  assert(strcmp(stmt.content.data_decl.segments[0].data.string, "Hello") == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_identifier_db_dec should handle number
void test_grammar_identifier_db_dec_number() {
  printf("Running test_grammar_identifier_db_dec_number...\n");

  // 65
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_NUMBER, "65", 60);
  tokens[1] = create_test_token(TOKEN_EOF, "", 60);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_DATA_DECL, 60);
  stmt.content.data_decl.type = DATA_BYTE;

  enum Err_Grm result = grammar_identifier_db_dec(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.data_decl.segments[0].type == INIT_SEG_VALUE);
  assert(stmt.content.data_decl.segments[0].data.value == 65);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_line_instruction should match instruction with no operands
void test_grammar_line_instruction_no_operands() {
  printf("Running test_grammar_line_instruction_no_operands...\n");

  // RET
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_INSTRUCTION, "RET", 65);
  tokens[1] = create_test_token(TOKEN_EOF, "", 65);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 65);

  enum Err_Grm result = grammar_line_instruction(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.type == STMT_INSTRUCTION);
  assert(stmt.content.instruction.operand_count == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_instruction_rhs should handle register operand
void test_grammar_instruction_rhs_register() {
  printf("Running test_grammar_instruction_rhs_register...\n");

  // A
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_REGISTER, "A", 70);
  tokens[1] = create_test_token(TOKEN_EOF, "", 70);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 70);

  enum Err_Grm result = grammar_instruction_rhs(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.instruction.operand_count == 1);
  assert(strcmp(stmt.content.instruction.operands[0].value.register_name,
                "A") == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_instruction_rhs should handle number operand
void test_grammar_instruction_rhs_number() {
  printf("Running test_grammar_instruction_rhs_number...\n");

  // 100
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_NUMBER, "100", 75);
  tokens[1] = create_test_token(TOKEN_EOF, "", 75);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 75);

  enum Err_Grm result = grammar_instruction_rhs(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.instruction.operand_count == 1);
  assert(stmt.content.instruction.operands[0].value.immediate_value == 100);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_instruction_rhs should handle label operand
void test_grammar_instruction_rhs_label() {
  printf("Running test_grammar_instruction_rhs_label...\n");

  // @loop
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_LABEL, "@loop", 80);
  tokens[1] = create_test_token(TOKEN_EOF, "", 80);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 80);

  enum Err_Grm result = grammar_instruction_rhs(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.instruction.operand_count == 1);
  assert(stmt.content.instruction.operands[0].specifier == OPS_LABEL);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_instruction_rhs_after should handle register
void test_grammar_instruction_rhs_after_register() {
  printf("Running test_grammar_instruction_rhs_after_register...\n");

  // B (as second operand)
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_REGISTER, "B", 85);
  tokens[1] = create_test_token(TOKEN_EOF, "", 85);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 85);
  stmt.content.instruction.operand_count = 1; // First operand already set

  enum Err_Grm result = grammar_instruction_rhs_after(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.instruction.operand_count == 2);
  assert(strcmp(stmt.content.instruction.operands[1].value.register_name,
                "B") == 0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_instruction_rhs_after should handle number
void test_grammar_instruction_rhs_after_number() {
  printf("Running test_grammar_instruction_rhs_after_number...\n");

  // 200 (as second operand)
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_NUMBER, "200", 90);
  tokens[1] = create_test_token(TOKEN_EOF, "", 90);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 90);
  stmt.content.instruction.operand_count = 1;

  enum Err_Grm result = grammar_instruction_rhs_after(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.instruction.operand_count == 2);
  assert(stmt.content.instruction.operands[1].value.immediate_value == 200);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

// Test: grammar_instruction_rhs_after should handle OFFSET identifier
void test_grammar_instruction_rhs_after_offset() {
  printf("Running test_grammar_instruction_rhs_after_offset...\n");

  // OFFSET myvar (as second operand)
  struct Token *tokens[3];
  tokens[0] = create_test_token(TOKEN_OFFSET, "OFFSET", 95);
  tokens[1] = create_test_token(TOKEN_IDENTIFIER, "myvar", 95);
  tokens[2] = create_test_token(TOKEN_EOF, "", 95);

  const struct Token *const_tokens[3] = {tokens[0], tokens[1], tokens[2]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_INSTRUCTION, 95);
  stmt.content.instruction.operand_count = 1;

  enum Err_Grm result = grammar_instruction_rhs_after(&stmt, const_tokens);

  assert(result == GRM_MATCH);
  assert(stmt.content.instruction.operand_count == 2);
  assert(stmt.content.instruction.operands[1].specifier == OPS_OFFSET);
  assert(strcmp(stmt.content.instruction.operands[1].value.label, "myvar") ==
         0);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 3);

  printf("  PASSED\n");
}

// Test: grammar functions should return GRM_NO_MATCH for invalid input
void test_grammar_error_handling() {
  printf("Running test_grammar_error_handling...\n");

  // Invalid: Just a comma
  struct Token *tokens[2];
  tokens[0] = create_test_token(TOKEN_COMMA, ",", 100);
  tokens[1] = create_test_token(TOKEN_EOF, "", 100);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};

  struct Parsed_Statement stmt;
  p_stmt_init(&stmt, STMT_NONE, 100);

  // Should not match any grammar rule
  enum Err_Grm result = grammar_line_kma(&stmt, const_tokens);
  assert(result == GRM_NO_MATCH);

  result = grammar_line_code(&stmt, const_tokens);
  assert(result == GRM_NO_MATCH);

  result = grammar_line_data(&stmt, const_tokens);
  assert(result == GRM_NO_MATCH);

  result = grammar_line_label(&stmt, const_tokens);
  assert(result == GRM_NO_MATCH);

  p_stmt_deinit(&stmt);
  cleanup_tokens(tokens, 2);

  printf("  PASSED\n");
}

int main(void) {
  printf("=== Running Parser Grammar Tests ===\n\n");

  test_grammar_line_empty();
  test_grammar_line_kma_match();
  test_grammar_line_kma_no_match();
  test_grammar_line_code_match();
  test_grammar_line_code_no_match();
  test_grammar_line_data_match();
  test_grammar_line_label_match();
  test_grammar_line_label_various_names();
  test_grammar_line_identifier_dword();
  test_grammar_line_identifier_byte();
  test_grammar_identifier_dw_dec_single();
  test_grammar_identifier_dw_dec_uninit();
  test_grammar_identifier_dw_dup_with_value();
  test_grammar_identifier_dw_dup_uninit();
  test_grammar_identifier_db_dec_string();
  test_grammar_identifier_db_dec_number();
  test_grammar_line_instruction_no_operands();
  test_grammar_instruction_rhs_register();
  test_grammar_instruction_rhs_number();
  test_grammar_instruction_rhs_label();
  test_grammar_instruction_rhs_after_register();
  test_grammar_instruction_rhs_after_number();
  test_grammar_instruction_rhs_after_offset();
  test_grammar_error_handling();

  printf("\n=== All Grammar Tests Passed! ===\n");
  return 0;
}
