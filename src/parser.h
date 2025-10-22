#ifndef PARSER_H
#define PARSER_H

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
// the EOF Token. If the operation fails, the Parsed Statement is returned with
// err different that PAR_NO_ERROR.
struct Parsed_Statement *parse_tokens(const struct Token *tokens, size_t nl);

// Free any Parsed Statement.
void parser_free(struct Parsed_Statement *stmt);

#endif
