#include "../src/instruction.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Utility for readable output
#define TEST(name) printf("\n=== %s ===\n", name)

int main(void) {
  printf("Running Instruction module tests...\n");

  // === Test 1: Mnemonic recognition ===
  TEST("instruction_is_mnemonic()");
  assert(instruction_is_mnemonic("MOV", 0) == 1);
  assert(instruction_is_mnemonic("ADD", 0) == 1);
  assert(instruction_is_mnemonic("XYZ", 0) == 0);
  assert(instruction_is_mnemonic("MOVSD", 0) == 1);
  assert(instruction_is_mnemonic(NULL, 0) == 0);

  // Test partial strings (with len)
  assert(instruction_is_mnemonic("MOV", 2) == 0);   // partial
  assert(instruction_is_mnemonic("MOVSD", 5) == 1); // exact
  printf("✅ Mnemonic checks passed.\n");

  // === Test 2: instruction_find() ===
  TEST("instruction_find()");
  const struct Instruction_Descriptor *desc;

  desc = instruction_find("MOV", 0, OP_REG, OP_IMM32);
  assert(desc != NULL);
  assert(strcmp(desc->mnemonic, "MOV") == 0);
  assert(desc->opcode == 0x10);
  assert(desc->operand1 == OP_REG && desc->operand2 == OP_IMM32);

  desc = instruction_find("MOV", 0, OP_REG, OP_REG);
  assert(desc != NULL);
  assert(desc->opcode == 0x11);

  // Should return NULL for invalid combo
  desc = instruction_find("MOV", 0, OP_IMM32, OP_REG);
  assert(desc == NULL);

  // Should handle non-existent mnemonic
  desc = instruction_find("XYZ", 0, OP_NONE, OP_NONE);
  assert(desc == NULL);
  printf("✅ instruction_find() tests passed.\n");

  // === Test 3: Encoded size ===
  TEST("instruction_get_encoded_size()");

  struct Instruction_Descriptor mock1 = {"FAKE1", 0x01, OP_NONE, OP_NONE, 0};
  struct Instruction_Descriptor mock2 = {"FAKE2", 0x02, OP_REG, OP_NONE, 1};
  struct Instruction_Descriptor mock3 = {"FAKE3", 0x03, OP_REG, OP_IMM32, 2};
  struct Instruction_Descriptor mock4 = {"FAKE4", 0x04, OP_IMM32, OP_IMM32, 2};

  assert(instruction_get_encoded_size(&mock1) == 1); // only opcode
  assert(instruction_get_encoded_size(&mock2) == 2); // opcode + reg
  assert(instruction_get_encoded_size(&mock3) == 6); // opcode + reg + imm32
  assert(instruction_get_encoded_size(&mock4) == 9); // opcode + imm32 + imm32

  assert(instruction_get_encoded_size(NULL) == 0); // null safety
  printf("✅ Encoded size tests passed.\n");

  // === Test 4: Real descriptor sanity check ===
  TEST("Real descriptor checks");
  const struct Instruction_Descriptor *add_reg_reg =
      instruction_find("ADD", 0, OP_REG, OP_REG);
  assert(add_reg_reg != NULL);
  assert(add_reg_reg->opcode == 0x31);
  assert(add_reg_reg->operand_count == 2);

  const struct Instruction_Descriptor *halt =
      instruction_find("HALT", 0, OP_NONE, OP_NONE);
  assert(halt != NULL);
  assert(halt->opcode == 0x00);
  assert(halt->operand_count == 0);
  printf("✅ Real descriptor tests passed.\n");

  printf("\n🎉 All Instruction tests passed successfully!\n");
  return 0;
}
