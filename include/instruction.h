#ifndef INSTRUCTION_H
#define INSTRUCTION_H
/* Module for defining all available instructions and a simple interface to work
 * with them. */

#include <stddef.h>
#include <stdint.h>

// Instruction has at most two operands each of which is of type:
enum Operand_Type {
  OP_NONE,  // The operand is not used
  OP_REG,   // Register
  OP_IMM32, // immediate 32 bit value
};

// Descriptor of available instructions - each found instruction in assembly
// .kas file has to map to some descriptor using its name, mnemonic.
struct Instruction_Descriptor {
  const char *mnemonic;       // Name used in assembly
  uint8_t opcode;             // 'Name' used in binary
  enum Operand_Type operand1; // Type of 1st operand
  enum Operand_Type operand2; // Type of 2nd operand
  int operand_count; // How many operands are used. viable values: 0,1,2
};

// Check if given <word> is an instruction mnemonic.
// If <len> == 0, <word> must be NULL-terminated, if <len> is specified,
// only that number of characters will be examined. Return 1 if <word> is
// instruction, 0 if not.
int instruction_is_mnemonic(const char *word, const size_t len);

// Find descriptor for given <mnemonic> and types of operands.
// If <len> == 0 <mnemonic> must be NULL-terminated, if <len> is specified,
// only that number of characters will be used from <mnemonic>. Return pointer
// to descriptor or NULL.
const struct Instruction_Descriptor *instruction_find(const char *mnemonic,
                                                      const size_t len,
                                                      enum Operand_Type op1,
                                                      enum Operand_Type op2);

// Calculate the size of an encoded instruction <desc>riptor in bytes,
size_t instruction_get_encoded_size(const struct Instruction_Descriptor *desc);

// If instruction <desc>riptor is a jump with relative offset, return 1.
// Otherwise return 0.
int instruction_is_relative_jump(const struct Instruction_Descriptor *desc);

#endif
