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

#endif
