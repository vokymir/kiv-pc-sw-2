#include "instruction.h"
#include <stddef.h>
#include <string.h>

// The complete instruction set of KM processor.
static const struct Instruction_Descriptor INSTRUCTION_TABLE[] = {
    // Control
    {"HALT", 0x00, OP_NONE, OP_NONE, 0},
    {"NOP", 0x90, OP_NONE, OP_NONE, 0},

    // Data movement
    {"MOV", 0x10, OP_REG, OP_IMM32, 2},
    {"MOV", 0x11, OP_REG, OP_REG, 2},
    {"MOVSD", 0x12, OP_REG, OP_REG, 2},
    {"LOAD", 0x13, OP_REG, OP_IMM32, 2},
    {"LOAD", 0x14, OP_REG, OP_REG, 2},
    {"STOR", 0x15, OP_REG, OP_IMM32, 2},
    {"STOR", 0x16, OP_REG, OP_REG, 2},

    // Stack
    {"PUSH", 0x20, OP_REG, OP_NONE, 1},
    {"POP", 0x21, OP_REG, OP_NONE, 1},

    // Arithmetic
    {"ADD", 0x30, OP_REG, OP_IMM32, 2},
    {"ADD", 0x31, OP_REG, OP_REG, 2},
    {"SUB", 0x32, OP_REG, OP_IMM32, 2},
    {"SUB", 0x33, OP_REG, OP_REG, 2},
    {"MUL", 0x34, OP_REG, OP_IMM32, 2},
    {"MUL", 0x35, OP_REG, OP_REG, 2},
    {"DIV", 0x36, OP_REG, OP_IMM32, 2},
    {"DIV", 0x37, OP_REG, OP_REG, 2},
    {"INC", 0x38, OP_REG, OP_NONE, 1},
    {"DEC", 0x39, OP_REG, OP_NONE, 1},

    // Logical
    {"AND", 0x40, OP_REG, OP_IMM32, 2},
    {"AND", 0x41, OP_REG, OP_REG, 2},
    {"OR", 0x42, OP_REG, OP_IMM32, 2},
    {"OR", 0x43, OP_REG, OP_REG, 2},
    {"XOR", 0x44, OP_REG, OP_IMM32, 2},
    {"XOR", 0x45, OP_REG, OP_REG, 2},
    {"NOT", 0x46, OP_REG, OP_NONE, 1},

    // Bit shifts
    {"SHL", 0x50, OP_REG, OP_IMM32, 2},
    {"SHL", 0x51, OP_REG, OP_REG, 2},
    {"SHR", 0x52, OP_REG, OP_IMM32, 2},
    {"SHR", 0x53, OP_REG, OP_REG, 2},

    // Compare
    {"CMP", 0x60, OP_REG, OP_IMM32, 2},
    {"CMP", 0x61, OP_REG, OP_REG, 2},

    // Jumps
    {"JMP", 0x70, OP_IMM32, OP_NONE, 1},
    {"JMP", 0x71, OP_REG, OP_NONE, 1},
    {"JE", 0x72, OP_IMM32, OP_NONE, 1},
    {"JNE", 0x73, OP_IMM32, OP_NONE, 1},
    {"JG", 0x74, OP_IMM32, OP_NONE, 1},
    {"JGE", 0x75, OP_IMM32, OP_NONE, 1},
    {"JNG", 0x76, OP_IMM32, OP_NONE, 1},
    {"JL", 0x77, OP_IMM32, OP_NONE, 1},
    {"JLE", 0x78, OP_IMM32, OP_NONE, 1},
    {"JNL", 0x79, OP_IMM32, OP_NONE, 1},

    // Subroutine
    {"CALL", 0x80, OP_IMM32, OP_NONE, 1},
    {"CALL", 0x81, OP_REG, OP_NONE, 1},
    {"RET", 0x82, OP_NONE, OP_NONE, 0},

    // I/O
    {"OUTD", 0xF0, OP_REG, OP_NONE, 1},
    {"OUTC", 0xF1, OP_REG, OP_NONE, 1},
    {"OUTS", 0xF2, OP_REG, OP_NONE, 1},
    {"INPD", 0xF3, OP_REG, OP_NONE, 1},
    {"INPC", 0xF4, OP_REG, OP_NONE, 1},
    {"INPS", 0xF5, OP_REG, OP_NONE, 1},
};

static const size_t INSTRUCTION_COUNT =
    sizeof(INSTRUCTION_TABLE) / sizeof(INSTRUCTION_TABLE[0]);

int instruction_is_mnemonic(const char *word, const size_t len) {
  size_t i = 0, i_len = 0, w_len = 0;
  if (!word) {
    return 0;
  }

  w_len = len;
  if (len == 0) {
    w_len = strlen(word);
  }

  for (i = 0; i < INSTRUCTION_COUNT; i++) {
    i_len = strlen(INSTRUCTION_TABLE[i].mnemonic);
    if (w_len != i_len) {
      continue; // skip different-length instructions
    }
    if (strncmp(INSTRUCTION_TABLE[i].mnemonic, word, w_len) == 0) {
      return 1; // found match
    }
  }

  return 0;
}

const struct Instruction_Descriptor *instruction_find(const char *mnemonic,
                                                      const size_t len,
                                                      enum Operand_Type op1,
                                                      enum Operand_Type op2) {
  size_t i = 0, i_len = 0, w_len = 0;
  if (!mnemonic) {
    return NULL;
  }

  w_len = len;
  if (len == 0) {
    w_len = strlen(mnemonic);
  }

  for (i = 0; i < INSTRUCTION_COUNT; i++) {
    i_len = strlen(INSTRUCTION_TABLE[i].mnemonic);
    if (w_len != i_len) {
      continue; // skip different-length instructions
    }
    if (strncmp(INSTRUCTION_TABLE[i].mnemonic, mnemonic, w_len) == 0 &&
        INSTRUCTION_TABLE[i].operand1 == op1 &&
        INSTRUCTION_TABLE[i].operand2 == op2) {
      return &INSTRUCTION_TABLE[i]; // Has the same mnemonic & both operands
    }
  }

  return NULL; // didn't found match
}

size_t instruction_get_encoded_size(const struct Instruction_Descriptor *desc) {
  size_t size = 1; // Always at least opcode byte
  if (!desc) {
    return 0;
  }

  if (desc->operand1 == OP_REG) {
    size += 1; // sizes of REG or immediate values are defined in assignment
  } else if (desc->operand1 == OP_IMM32) {
    size += 4;
  }

  if (desc->operand2 == OP_REG) {
    size += 1;
  } else if (desc->operand2 == OP_IMM32) {
    size += 4;
  }

  return size;
}
