#include "../src/lexer.h"
#include "../src/memory.h"
#include "../src/parser.h"
#include "../src/parser_code.h"
#include "../src/parser_data.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to create a token for testing
struct Token *create_token(enum Token_Type type, const char *value,
                           size_t line) {
  struct Token *tok = jalloc(sizeof(struct Token));
  tok->type = type;
  strncpy(tok->value, value, TOKEN_MAX_VALUE_LEN - 1);
  tok->value[TOKEN_MAX_VALUE_LEN - 1] = '\0';
  tok->line_number = line;
  return tok;
}

// Helper to free token array
void free_token_array(struct Token **tokens, size_t count) {
  for (size_t i = 0; i < count; i++) {
    jree(tokens[i]);
  }
}

// Test: p_stmt_create should return valid pointer for valid statement types
void test_p_stmt_create_valid_types() {
  printf("Running test_p_stmt_create_valid_types...\n");

  // Test creating each statement type
  struct Parsed_Statement *stmt;

  stmt = p_stmt_create(STMT_NONE, 1);
  assert(stmt != NULL);
  assert(stmt->type == STMT_NONE);
  assert(stmt->line_number == 1);
  p_stmt_free(&stmt);
  assert(stmt == NULL);

  stmt = p_stmt_create(STMT_KMA, 2);
  assert(stmt != NULL);
  assert(stmt->type == STMT_KMA);
  assert(stmt->line_number == 2);
  p_stmt_free(&stmt);

  stmt = p_stmt_create(STMT_SECTION_DATA, 3);
  assert(stmt != NULL);
  assert(stmt->type == STMT_SECTION_DATA);
  p_stmt_free(&stmt);

  stmt = p_stmt_create(STMT_SECTION_CODE, 4);
  assert(stmt != NULL);
  assert(stmt->type == STMT_SECTION_CODE);
  p_stmt_free(&stmt);

  stmt = p_stmt_create(STMT_LABEL_DEF, 5);
  assert(stmt != NULL);
  assert(stmt->type == STMT_LABEL_DEF);
  p_stmt_free(&stmt);

  stmt = p_stmt_create(STMT_DATA_DECL, 6);
  assert(stmt != NULL);
  assert(stmt->type == STMT_DATA_DECL);
  p_stmt_free(&stmt);

  stmt = p_stmt_create(STMT_INSTRUCTION, 7);
  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  p_stmt_free(&stmt);

  printf("  PASSED\n");
}

// Test: p_stmt_init should properly initialize based on type
void test_p_stmt_init() {
  printf("Running test_p_stmt_init...\n");

  struct Parsed_Statement stmt;
  int result;

  // Initialize as STMT_NONE
  result = p_stmt_init(&stmt, STMT_NONE, 1);
  assert(result == 1);
  assert(stmt.type == STMT_NONE);
  assert(stmt.err == PAR_NO_ERROR);
  p_stmt_deinit(&stmt);

  // Initialize as STMT_DATA_DECL
  result = p_stmt_init(&stmt, STMT_DATA_DECL, 2);
  assert(result == 1);
  assert(stmt.type == STMT_DATA_DECL);
  // Should initialize segments pointer to NULL initially
  assert(stmt.content.data_decl.segments == NULL);
  assert(stmt.content.data_decl.segment_count == 0);
  p_stmt_deinit(&stmt);

  // Initialize as STMT_INSTRUCTION
  result = p_stmt_init(&stmt, STMT_INSTRUCTION, 3);
  assert(result == 1);
  assert(stmt.type == STMT_INSTRUCTION);
  assert(stmt.content.instruction.operand_count == 0);
  p_stmt_deinit(&stmt);

  printf("  PASSED\n");
}

// Test: p_stmt_deinit should properly clean up resources
void test_p_stmt_deinit() {
  printf("Running test_p_stmt_deinit...\n");

  struct Parsed_Statement stmt;

  // Create a data declaration with segments
  p_stmt_init(&stmt, STMT_DATA_DECL, 1);
  stmt.content.data_decl.segment_count = 3;
  stmt.content.data_decl.segments = jalloc(3 * sizeof(struct Init_Segment));

  // Deinit should free segments and zero out everything
  p_stmt_deinit(&stmt);
  assert(stmt.type == STMT_NONE);
  assert(stmt.content.data_decl.segments == NULL);
  assert(stmt.content.data_decl.segment_count == 0);

  printf("  PASSED\n");
}

// Test: parse_tokens with empty line (only EOF)
void test_parse_tokens_empty_line() {
  printf("Running test_parse_tokens_empty_line...\n");

  struct Token *tokens[1];
  tokens[0] = create_token(TOKEN_EOF, "", 1);

  const struct Token *const_tokens[1] = {tokens[0]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 1);

  assert(stmt != NULL);
  assert(stmt->type == STMT_NONE);
  assert(stmt->line_number == 1);

  p_stmt_free(&stmt);
  free_token_array(tokens, 1);

  printf("  PASSED\n");
}

// Test: parse_tokens with KMA directive
void test_parse_tokens_kma() {
  printf("Running test_parse_tokens_kma...\n");

  struct Token *tokens[2];
  tokens[0] = create_token(TOKEN_KMA, ".KMA", 1);
  tokens[1] = create_token(TOKEN_EOF, "", 1);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 1);

  assert(stmt != NULL);
  assert(stmt->type == STMT_KMA);
  assert(stmt->err == PAR_NO_ERROR);

  p_stmt_free(&stmt);
  free_token_array(tokens, 2);

  printf("  PASSED\n");
}

// Test: parse_tokens with .DATA section
void test_parse_tokens_section_data() {
  printf("Running test_parse_tokens_section_data...\n");

  struct Token *tokens[2];
  tokens[0] = create_token(TOKEN_SECTION_DATA, ".DATA", 5);
  tokens[1] = create_token(TOKEN_EOF, "", 5);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 5);

  assert(stmt != NULL);
  assert(stmt->type == STMT_SECTION_DATA);
  assert(stmt->line_number == 5);

  p_stmt_free(&stmt);
  free_token_array(tokens, 2);

  printf("  PASSED\n");
}

// Test: parse_tokens with .CODE section
void test_parse_tokens_section_code() {
  printf("Running test_parse_tokens_section_code...\n");

  struct Token *tokens[2];
  tokens[0] = create_token(TOKEN_SECTION_CODE, ".CODE", 10);
  tokens[1] = create_token(TOKEN_EOF, "", 10);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 10);

  assert(stmt != NULL);
  assert(stmt->type == STMT_SECTION_CODE);
  assert(stmt->line_number == 10);

  p_stmt_free(&stmt);
  free_token_array(tokens, 2);

  printf("  PASSED\n");
}

// Test: parse_tokens with label definition
void test_parse_tokens_label() {
  printf("Running test_parse_tokens_label...\n");

  struct Token *tokens[2];
  tokens[0] = create_token(TOKEN_LABEL, "@start", 15);
  tokens[1] = create_token(TOKEN_EOF, "", 15);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 15);

  assert(stmt != NULL);
  assert(stmt->type == STMT_LABEL_DEF);
  assert(stmt->line_number == 15);
  assert(strcmp(stmt->content.label_def.label_name, "@start") == 0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 2);

  printf("  PASSED\n");
}

// Test: parse_tokens with simple data declaration (DWORD with single value)
void test_parse_tokens_data_decl_simple() {
  printf("Running test_parse_tokens_data_decl_simple...\n");

  // var1 DWORD 42
  struct Token *tokens[4];
  tokens[0] = create_token(TOKEN_IDENTIFIER, "var1", 20);
  tokens[1] = create_token(TOKEN_DATA_TYPE, "DWORD", 20);
  tokens[2] = create_token(TOKEN_NUMBER, "42", 20);
  tokens[3] = create_token(TOKEN_EOF, "", 20);

  const struct Token *const_tokens[4] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 20);

  assert(stmt != NULL);
  assert(stmt->type == STMT_DATA_DECL);
  assert(stmt->line_number == 20);
  assert(strcmp(stmt->content.data_decl.identifier, "var1") == 0);
  assert(stmt->content.data_decl.type == DATA_DWORD);
  assert(stmt->content.data_decl.segment_count == 1);
  assert(stmt->content.data_decl.segments[0].type == INIT_SEG_VALUE);
  assert(stmt->content.data_decl.segments[0].data.value == 42);
  assert(stmt->content.data_decl.segments[0].is_uninit == 0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 4);

  printf("  PASSED\n");
}

// Test: parse_tokens with data declaration using uninitialized marker (?)
void test_parse_tokens_data_decl_uninit() {
  printf("Running test_parse_tokens_data_decl_uninit...\n");

  // var2 DWORD ?
  struct Token *tokens[4];
  tokens[0] = create_token(TOKEN_IDENTIFIER, "var2", 25);
  tokens[1] = create_token(TOKEN_DATA_TYPE, "DWORD", 25);
  tokens[2] = create_token(TOKEN_QUESTION, "?", 25);
  tokens[3] = create_token(TOKEN_EOF, "", 25);

  const struct Token *const_tokens[4] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 25);

  assert(stmt != NULL);
  assert(stmt->type == STMT_DATA_DECL);
  assert(stmt->content.data_decl.segments[0].is_uninit == 1);
  assert(stmt->content.data_decl.is_fully_uninit == 1);

  p_stmt_free(&stmt);
  free_token_array(tokens, 4);

  printf("  PASSED\n");
}

// Test: parse_tokens with data declaration using DUP
void test_parse_tokens_data_decl_dup() {
  printf("Running test_parse_tokens_data_decl_dup...\n");

  // array DWORD 10 DUP(0)
  struct Token *tokens[8];
  tokens[0] = create_token(TOKEN_IDENTIFIER, "array", 30);
  tokens[1] = create_token(TOKEN_DATA_TYPE, "DWORD", 30);
  tokens[2] = create_token(TOKEN_NUMBER, "10", 30);
  tokens[3] = create_token(TOKEN_DUP, "DUP", 30);
  tokens[4] = create_token(TOKEN_LPAREN, "(", 30);
  tokens[5] = create_token(TOKEN_NUMBER, "0", 30);
  tokens[6] = create_token(TOKEN_RPAREN, ")", 30);
  tokens[7] = create_token(TOKEN_EOF, "", 30);

  const struct Token *const_tokens[8] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4], tokens[5],
                                         tokens[6], tokens[7]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 30);

  assert(stmt != NULL);
  assert(stmt->type == STMT_DATA_DECL);
  assert(stmt->content.data_decl.segments[0].type == INIT_SEG_DUP);
  assert(stmt->content.data_decl.segments[0].data.dup.count == 10);
  assert(stmt->content.data_decl.segments[0].data.dup.value == 0);
  assert(stmt->content.data_decl.segments[0].element_count == 10);

  p_stmt_free(&stmt);
  free_token_array(tokens, 8);

  printf("  PASSED\n");
}

// Test: parse_tokens with multiple data values (comma-separated)
void test_parse_tokens_data_decl_multiple() {
  printf("Running test_parse_tokens_data_decl_multiple...\n");

  // values DWORD 1, 2, 3
  struct Token *tokens[8];
  tokens[0] = create_token(TOKEN_IDENTIFIER, "values", 35);
  tokens[1] = create_token(TOKEN_DATA_TYPE, "DWORD", 35);
  tokens[2] = create_token(TOKEN_NUMBER, "1", 35);
  tokens[3] = create_token(TOKEN_COMMA, ",", 35);
  tokens[4] = create_token(TOKEN_NUMBER, "2", 35);
  tokens[5] = create_token(TOKEN_COMMA, ",", 35);
  tokens[6] = create_token(TOKEN_NUMBER, "3", 35);
  tokens[7] = create_token(TOKEN_EOF, "", 35);

  const struct Token *const_tokens[8] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4], tokens[5],
                                         tokens[6], tokens[7]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 35);

  assert(stmt != NULL);
  assert(stmt->type == STMT_DATA_DECL);
  assert(stmt->content.data_decl.segment_count == 3);
  assert(stmt->content.data_decl.segments[0].data.value == 1);
  assert(stmt->content.data_decl.segments[1].data.value == 2);
  assert(stmt->content.data_decl.segments[2].data.value == 3);

  p_stmt_free(&stmt);
  free_token_array(tokens, 8);

  printf("  PASSED\n");
}

// Test: parse_tokens with BYTE string
void test_parse_tokens_data_decl_string() {
  printf("Running test_parse_tokens_data_decl_string...\n");

  // msg BYTE "Hello"
  struct Token *tokens[4];
  tokens[0] = create_token(TOKEN_IDENTIFIER, "msg", 40);
  tokens[1] = create_token(TOKEN_DATA_TYPE, "BYTE", 40);
  tokens[2] = create_token(TOKEN_STRING, "Hello", 40);
  tokens[3] = create_token(TOKEN_EOF, "", 40);

  const struct Token *const_tokens[4] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 40);

  assert(stmt != NULL);
  assert(stmt->type == STMT_DATA_DECL);
  assert(stmt->content.data_decl.type == DATA_BYTE);
  assert(stmt->content.data_decl.segments[0].type == INIT_SEG_STRING);
  assert(strcmp(stmt->content.data_decl.segments[0].data.string, "Hello") == 0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 4);

  printf("  PASSED\n");
}

// Test: parse_tokens with simple instruction (no operands)
void test_parse_tokens_instruction_no_operands() {
  printf("Running test_parse_tokens_instruction_no_operands...\n");

  // RET
  struct Token *tokens[2];
  tokens[0] = create_token(TOKEN_INSTRUCTION, "RET", 50);
  tokens[1] = create_token(TOKEN_EOF, "", 50);

  const struct Token *const_tokens[2] = {tokens[0], tokens[1]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 50);

  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  assert(stmt->content.instruction.operand_count == 0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 2);

  printf("  PASSED\n");
}

// Test: parse_tokens with instruction with register operand
void test_parse_tokens_instruction_one_register() {
  printf("Running test_parse_tokens_instruction_one_register...\n");

  // PUSH A
  struct Token *tokens[3];
  tokens[0] = create_token(TOKEN_INSTRUCTION, "PUSH", 55);
  tokens[1] = create_token(TOKEN_REGISTER, "A", 55);
  tokens[2] = create_token(TOKEN_EOF, "", 55);

  const struct Token *const_tokens[3] = {tokens[0], tokens[1], tokens[2]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 55);

  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  assert(stmt->content.instruction.operand_count == 1);
  assert(strcmp(stmt->content.instruction.operands[0].value.register_name,
                "A") == 0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 3);

  printf("  PASSED\n");
}

// Test: parse_tokens with instruction with two register operands
void test_parse_tokens_instruction_two_registers() {
  printf("Running test_parse_tokens_instruction_two_registers...\n");

  // MOV A, B
  struct Token *tokens[5];
  tokens[0] = create_token(TOKEN_INSTRUCTION, "MOV", 60);
  tokens[1] = create_token(TOKEN_REGISTER, "A", 60);
  tokens[2] = create_token(TOKEN_COMMA, ",", 60);
  tokens[3] = create_token(TOKEN_REGISTER, "B", 60);
  tokens[4] = create_token(TOKEN_EOF, "", 60);

  const struct Token *const_tokens[5] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 60);

  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  printf("op_count %i", stmt->content.instruction.operand_count);
  fflush(stdout);
  assert(stmt->content.instruction.operand_count == 2);
  assert(strcmp(stmt->content.instruction.operands[0].value.register_name,
                "A") == 0);
  assert(strcmp(stmt->content.instruction.operands[1].value.register_name,
                "B") == 0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 5);

  printf("  PASSED\n");
}

// Test: parse_tokens with instruction with register and immediate
void test_parse_tokens_instruction_reg_immediate() {
  printf("Running test_parse_tokens_instruction_reg_immediate...\n");

  // ADD A, 100
  struct Token *tokens[5];
  tokens[0] = create_token(TOKEN_INSTRUCTION, "ADD", 65);
  tokens[1] = create_token(TOKEN_REGISTER, "A", 65);
  tokens[2] = create_token(TOKEN_COMMA, ",", 65);
  tokens[3] = create_token(TOKEN_NUMBER, "100", 65);
  tokens[4] = create_token(TOKEN_EOF, "", 65);

  const struct Token *const_tokens[5] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 65);

  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  assert(stmt->content.instruction.operand_count == 2);
  assert(stmt->content.instruction.operands[1].value.immediate_value == 100);

  p_stmt_free(&stmt);
  free_token_array(tokens, 5);

  printf("  PASSED\n");
}

// Test: parse_tokens with instruction with OFFSET identifier
void test_parse_tokens_instruction_offset() {
  printf("Running test_parse_tokens_instruction_offset...\n");

  // MOV A, OFFSET myvar
  struct Token *tokens[6];
  tokens[0] = create_token(TOKEN_INSTRUCTION, "MOV", 70);
  tokens[1] = create_token(TOKEN_REGISTER, "A", 70);
  tokens[2] = create_token(TOKEN_COMMA, ",", 70);
  tokens[3] = create_token(TOKEN_OFFSET, "OFFSET", 70);
  tokens[4] = create_token(TOKEN_IDENTIFIER, "myvar", 70);
  tokens[5] = create_token(TOKEN_EOF, "", 70);

  const struct Token *const_tokens[6] = {tokens[0], tokens[1], tokens[2],
                                         tokens[3], tokens[4], tokens[5]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 70);

  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  assert(stmt->content.instruction.operand_count == 2);
  assert(stmt->content.instruction.operands[1].specifier == OPS_OFFSET);
  assert(strcmp(stmt->content.instruction.operands[1].value.label, "myvar") ==
         0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 6);

  printf("  PASSED\n");
}

// Test: parse_tokens with instruction with label reference
void test_parse_tokens_instruction_label() {
  printf("Running test_parse_tokens_instruction_label...\n");

  // JMP @loop
  struct Token *tokens[3];
  tokens[0] = create_token(TOKEN_INSTRUCTION, "JMP", 75);
  tokens[1] = create_token(TOKEN_LABEL, "@loop", 75);
  tokens[2] = create_token(TOKEN_EOF, "", 75);

  const struct Token *const_tokens[3] = {tokens[0], tokens[1], tokens[2]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 75);

  assert(stmt != NULL);
  assert(stmt->type == STMT_INSTRUCTION);
  assert(stmt->content.instruction.operand_count == 1);
  assert(stmt->content.instruction.operands[0].specifier == OPS_LABEL);
  assert(strcmp(stmt->content.instruction.operands[0].value.label, "@loop") ==
         0);

  p_stmt_free(&stmt);
  free_token_array(tokens, 3);

  printf("  PASSED\n");
}

// Test: parse_tokens returns NULL for invalid input
void test_parse_tokens_invalid_null() {
  printf("Running test_parse_tokens_invalid_null...\n");

  // NULL pointer should return NULL
  struct Parsed_Statement *stmt = parse_tokens(NULL, 0);
  assert(stmt == NULL);

  printf("  PASSED\n");
}

// Test: parse_tokens with error - statement should have error flag
void test_parse_tokens_error_handling() {
  printf("Running test_parse_tokens_error_handling...\n");

  // Invalid token sequence that should produce error
  struct Token *tokens[3];
  tokens[0] = create_token(TOKEN_COMMA, ",", 80);
  tokens[1] = create_token(TOKEN_COMMA, ",", 80);
  tokens[2] = create_token(TOKEN_EOF, "", 80);

  const struct Token *const_tokens[3] = {tokens[0], tokens[1], tokens[2]};
  struct Parsed_Statement *stmt = parse_tokens(const_tokens, 80);

  // Should either return NULL or return statement with ERROR type
  if (stmt != NULL) {
    assert(stmt->type == STMT_ERROR || stmt->err != PAR_NO_ERROR);
    p_stmt_free(&stmt);
  }

  free_token_array(tokens, 3);

  printf("  PASSED\n");
}

int main(void) {
  printf("=== Running Parser Tests ===\n\n");

  test_p_stmt_create_valid_types();
  test_p_stmt_init();
  test_p_stmt_deinit();
  test_parse_tokens_empty_line();
  test_parse_tokens_kma();
  test_parse_tokens_section_data();
  test_parse_tokens_section_code();
  test_parse_tokens_label();
  test_parse_tokens_data_decl_simple();
  test_parse_tokens_data_decl_uninit();
  test_parse_tokens_data_decl_dup();
  test_parse_tokens_data_decl_multiple();
  test_parse_tokens_data_decl_string();
  test_parse_tokens_instruction_no_operands();
  test_parse_tokens_instruction_one_register();
  test_parse_tokens_instruction_two_registers();
  test_parse_tokens_instruction_reg_immediate();
  test_parse_tokens_instruction_offset();
  test_parse_tokens_instruction_label();
  test_parse_tokens_invalid_null();
  test_parse_tokens_error_handling();

  printf("\n=== All Parser Tests Passed! ===\n");
  return 0;
}
