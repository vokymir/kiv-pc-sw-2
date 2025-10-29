#ifndef PARSER_CODE_H
#define PARSER_CODE_H

#include "instruction.h"

#define MAX_REGISTER_NAME_LEN 4
#define MAX_LABEL_NAME_LEN 256

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
  struct Operand operands[2];
  const struct Instruction_Descriptor *descriptor; // equivalent descriptor
};

struct Label_Definition {
  char label_name[MAX_LABEL_NAME_LEN]; // The label name including @
};

// Allocate new Instruction statement and call i_stmt_init to inicialize
// insides. Return pointer on success, NULL on failure.
struct Instruction_Statement *
i_stmt_create(int op_count, const struct Operand *op1,
              const struct Operand *op2,
              const struct Instruction_Descriptor *id);

// Initialize existing Instruction statement from paramters.
// If any parameter pointer is NULL it is initialized as empty.
// Return 1 on success, 0 on failure.
int i_stmt_init(struct Instruction_Statement *istmt, int op_count,
                const struct Operand *op1, const struct Operand *op2,
                const struct Instruction_Descriptor *id);

// Clear all insides of Instruction Statement.
void i_stmt_deinit(struct Instruction_Statement *istmt);

// Free dynamically allocated Instruction statement.
// On success, set *istmt = NULL
void i_stmt_free(struct Instruction_Statement **istmt);

// Dynamically allocate Label Definition.
// Call labdef_init to initialize.
// Return pointer on NULL.
struct Label_Definition *labdef_create(char *name);

// Initialize given Label Definition.
// Accepts NULL ptrs, in that case uninitialize that attribute.
// Return 1 on success, 0 on failure.
int labdef_init(struct Label_Definition *ld, char *name);

// Clear all Label Definition insides.
void labdef_deinit(struct Label_Definition *ld);

// Free dynamically allocated Label Definition and before it call labdef_deinit.
// On success set *ld = NULL
void labdef_free(struct Label_Definition **ld);

#endif
