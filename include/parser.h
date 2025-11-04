#ifndef PARSER_H
#define PARSER_H

#include <stddef.h>
#include <stdint.h>

#include "instruction.h"
#include "lexer.h"

// TEMPORARY TODO: organize parser header file to only contain public thingies
// and all private go to src/parser/internal.h or whatever

// instruction table doesn't know offset & label, but in kma-assembly it
// sometimes is. this is a way to count for it
enum Operand_Specifier {
  OPS_NONE = 0, // type is adequate
  OPS_OFFSET,   // type is imm32, but really is an offset
  OPS_LABEL,    // type is imm32, but really is a label
};

// in an instruction
struct Operand {
  enum Operand_Type type;
  enum Operand_Specifier specifier;
  union {
    char register_name[MAX_REGISTER_NAME_LEN]; // A, B, C,...
    int32_t immediate_value;
    char label[MAX_LABEL_NAME_LEN]; // Name of label/variable being referenced
  } value;
};

struct Instruction_Statement {
  int operand_count;
  struct Operand operands[KMA_MAX_OPERANDS];
  const struct Instruction_Descriptor *descriptor; // equivalent descriptor
};

struct Label_Definition {
  char label_name[MAX_LABEL_NAME_LEN]; // The label name including @
};

enum Data_Type { DATA_DWORD, DATA_BYTE, DATA_ERROR };

enum Init_Segment_Type {
  INIT_SEG_VALUE,
  INIT_SEG_DUP,
  INIT_SEG_STRING,
  INIT_SEG_UNINIT
};

// one segment of data declaration
struct Init_Segment {
  enum Init_Segment_Type type;
  union {
    int32_t value; // number
    struct {
      size_t count;                           // how many
      int32_t value;                          // number - or ? if is_uninit
    } dup;                                    // dup
    char string[MAX_INIT_SEGMENT_STRING_LEN]; // string
  } data;
  size_t element_count; // length of string/count in dup/1 for number
  int is_uninit;        // is value/dup un-initialized
};

// when delaring an assembler variable
struct Data_Declaration {
  char identifier[MAX_IDENTIFIER_LEN]; // name of variable
  enum Data_Type type;

  struct Init_Segment *segments; // array of segments
  size_t segment_count;

  size_t total_size;   // total size of all segments->element_count
  int is_fully_uninit; // if 1 if and only if every init segment is_uninit
};

enum Err_Parse {
  PAR_NO_ERROR = 0,
  PAR_EMPTY_LINE = 1,
};

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
