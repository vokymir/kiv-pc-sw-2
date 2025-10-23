#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdint.h>

#include "lexer.h"
#include "parser_code.h"
#include "parser_data.h"

enum Err_Parse {
  PAR_NO_ERROR = 0,
};

enum Statement_Type {
  STMT_NONE,         // Empty line or comment-only
  STMT_SECTION_DATA, // .DATA
  STMT_SECTION_CODE, // .CODE
  STMT_LABEL_DEF,    // Label definition
  STMT_DATA_DECL,    // Data declaration
  STMT_INSTRUCTION,  // An instruction
  STMT_ERROR         // Parse error
};

struct Parsed_Statement {
  enum Statement_Type type;
  enum Err_Parse err;
  size_t line_number; // Which line this came from

  union {
    struct Instruction_Statement instruction;
    struct Data_Declaration data_decl;
    struct Label_Definition label_def;
  } content;
};

// Create new Parsed Statement from array of Tokens. This array MUST be ended by
// the EOF Token. If the operation fails, NULL is returned. If the Parsed
// Statement is returned with err different that PAR_NO_ERROR, than read the
// error.
struct Parsed_Statement *parse_tokens(const struct Token *tokens[], size_t nl);

// Create new Parsed Statement and initializes the content by calling
// p_stmt_init. Return pointer or NULL.
struct Parsed_Statement *p_stmt_create(enum Statement_Type stype, size_t nl);

// Initialize the Parsed Statements insides based on type,
// return 1 on success, 0 on failure.
int p_stmt_init(struct Parsed_Statement *ps, enum Statement_Type type,
                size_t nl);

// Free all parser insides, set every variable/pointer to 0.
void p_stmt_deinit(struct Parsed_Statement *ps);

// Free any Parsed Statement dynamically allocated.
// Calls p_stmt_deinit before freeing.
// Set *stmt = NULL on success
void p_stmt_free(struct Parsed_Statement **stmt);

#endif
