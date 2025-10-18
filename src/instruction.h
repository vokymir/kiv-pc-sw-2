#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stddef.h>
#include <stdint.h>

// Operand type for instruction
enum Operand_Type {
  OP_NONE,
  OP_REG,
  OP_IMM32, // immediate 32 bit value
  OP_REG_OR_IMM32
};

struct Instruction_Descriptor {
  const char *mnemonic; // name used in assembly
  uint8_t opcode;
  enum Operand_Type operand1;
  enum Operand_Type operand2;
  int operand_count; // 0,1,2
};

// Check if given string is an instruction mnemonic.
// If len == 0 word must be NULL-terminated, if len is specified,
// uses it inside strncpy. Return 1 if is instruction, 0 if not.
int instruction_is_mnemonic(const char *word, const size_t len);

// Find I_D for given mnemonic and types of operands.
// If len == 0 word must be NULL-terminated, if len is specified,
// uses it inside strncpy. Return pointer to descriptor or NULL.
const struct Instruction_Descriptor *instruction_find(const char *mnemonic,
                                                      const size_t len,
                                                      enum Operand_Type op1,
                                                      enum Operand_Type op2);

// Calculate the size of an encoded instruction in bytes,
size_t instruction_get_encoded_size(const struct Instruction_Descriptor *desc);

#endif
