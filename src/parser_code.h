#ifndef PARSER_CODE_H
#define PARSER_CODE_H

#include "instruction.h"

#define MAX_REGISTER_NAME_LEN 4
#define MAX_LABEL_NAME_LEN 256

// in an instruction
struct Operand {
  Operand_Type type;
  union {
    char register_name[MAX_REGISTER_NAME_LEN]; // A, B, C,...
    int32_t immediate_value;
    char *label; // Name of label/variable being referenced
  } value;
};

struct Instruction_Statement {
  int operand_count;
  Operand operands[2];
  const Instruction_Descriptor *descriptor; // equivalent descriptor
};

struct Label_Definition {
  char label_name[MAX_LABEL_NAME_LEN]; // The label name including @
};

#endif
