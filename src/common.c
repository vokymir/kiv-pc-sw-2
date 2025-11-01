#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "instruction.h"

void print_verbose(int condition, const char *string, ...) {
  if (!condition) {
    return;
  }

  printf("[VERBOSE] ");
  va_list args;
  va_start(args, string);
  vprintf(string, args);
  va_end(args);
}

void print_verbose_clean(int condition, const char *string, ...) {
  if (!condition) {
    return;
  }

  va_list args;
  va_start(args, string);
  vprintf(string, args);
  va_end(args);
}

void print_instruction(int condition, size_t line,
                       struct Instruction_Statement *is, size_t addr) {
  if (!condition || !is || !is->descriptor || !is->descriptor->mnemonic) {
    return;
  }

  // TODO: nicely output the operands...
  printf("[INSTR] L%zu: %s %s %s at CS:%zu\n", line, is->descriptor->mnemonic,
         (is->operands[0].type != OP_NONE) ? "OP1" : "",
         (is->operands[1].type != OP_NONE) ? "OP2" : "", addr);
}
