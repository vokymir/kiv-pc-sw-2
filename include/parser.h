#ifndef PARSER_H
#define PARSER_H
/* Module for parsing Token[] given by lexer and transforming that into Parsed
 * Statement with all information stored in it. */

#include <stddef.h>
#include <stdint.h>

#include "instruction.h"
#include "lexer.h"

// 1================
// ===== ENUMS =====
// =================

// Errors encoutered while parsing a line
// Maybe redundant source of error information.
enum Err_Parse {
  PAR_NO_ERROR = 0,
  PAR_EMPTY_LINE = 1,
};

// On one line (= one token array) is exactly one statement. Be it label
// definition, data declaration, instruction or other
enum Statement_Type {
  STMT_NONE,         // Empty line or comment-only
  STMT_KMA,          // defines assembler file
  STMT_SECTION_DATA, // .DATA
  STMT_SECTION_CODE, // .CODE
  STMT_LABEL_DEF,    // Label definition
  STMT_DATA_DECL,    // Data declaration
  STMT_INSTRUCTION,  // An instruction
  STMT_ERROR         // Parse error
};

// KMA computer supports these Data types
enum Data_Type { DATA_DWORD, DATA_BYTE };

// When declaring data, the declaration is a consecutive array of segments, each
// of which can be of a different type.
enum Init_Segment_Type {
  INIT_SEG_VALUE,  // immediate 32 bit value
  INIT_SEG_DUP,    // DUP which 'create array'
  INIT_SEG_STRING, // string in double-quotes
  INIT_SEG_UNINIT  // uninitialized value
};

// If operand specified in Instruction Descriptor is of type IMM32, in assembly
// .kas file it can be represented in three ways:
enum Operand_Specifier {
  OPS_NONE = 0, // really is just IMM32, stored in valud.immediate_value
  OPS_OFFSET,   // is position of variable in data segment. The variable name is
                // stored in value.label
  OPS_LABEL, // is a position of label in code segment. The label name is stored
             // in value.label
};

// 2==================
// ===== STRUCTS =====
// ===================

// Specify abstract type of operand to one with value. Can have only value at a
// time based on its <type> and <specifier>.
struct Operand {
  enum Operand_Type type;
  enum Operand_Specifier specifier;
  union {
    char register_name[MAX_REGISTER_NAME_LEN]; // A, B, C,...
    int32_t immediate_value;
    char label[MAX_LABEL_NAME_LEN]; // Name of label/variable being referenced
  } value;
};

// Representation of one instruction line in assembly .kas. It is a known
// instruction with <descriptor> and has specific <operands>.
struct Instruction_Statement {
  int operand_count;
  struct Operand operands[KMA_MAX_OPERANDS];
  const struct Instruction_Descriptor *descriptor; // equivalent descriptor
};

// Representation of one line in code segment on which a label is defined.
struct Label_Definition {
  char label_name[MAX_LABEL_NAME_LEN]; // The label name including @
};

// The variable declared and initialized in data section can consist of multiple
// initializing segments separated by comma. Each segment can be of different
// type. This struct represents one segment in the declaration.
struct Init_Segment {
  enum Init_Segment_Type type;
  union {
    int32_t value; // number
    struct {
      size_t count;                           // how many times duplicate
      int32_t value;                          // duplicate this (number - or ?)
    } dup;                                    // duplicate
    char string[MAX_INIT_SEGMENT_STRING_LEN]; // string
  } data;
  size_t element_count; // length of string/count in dup/1 for number
  int is_uninit;        // is value/dup un-initialized
};

// Declaration of variable in Data section.
struct Data_Declaration {
  char identifier[MAX_IDENTIFIER_LEN]; // Name of variable
  enum Data_Type type;                 // byte/dword

  struct Init_Segment *segments; // array of segments
  size_t segment_count;          // how many

  size_t total_size; // total size of all segments->element_count
                     // WARNING: total size must be multiplied with the size of
                     // used data type to get real size in data segment

  int is_fully_uninit; // is 1 if and only if every init segment is_uninit
};

// Every parsed line produces one Parsed Statement representation.
// The abbreviation PSTMT standing for Parsed StaTeMenT is used.
struct Parsed_Statement {
  enum Statement_Type type;
  enum Err_Parse err; // If during parsing occured error.
  size_t line_number; // Which line this statement originates at

  union {
    struct Instruction_Statement instruction;
    struct Data_Declaration data_decl;
    struct Label_Definition label_def;
  } content; // determined by <type>
};

// 3================================
// ===== FUNCTION DECLARATIONS =====
// =================================

// Create new Parsed Statement from array of Tokens. This array MUST be ended by
// the EOF Token. If the operation fails, NULL is returned. If the Parsed
// Statement is returned with err different that PAR_NO_ERROR, than read the
// <err> in Parsed Statement.
struct Parsed_Statement *parse_tokens(const struct Token *tokens[], size_t nl);

// Create new Parsed Statement and initializes the content by calling
// p_stmt_init. Return pointer to new pstmt or NULL.
// Created pstmt must be freed using p_stmt_free
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
