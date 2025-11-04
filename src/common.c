#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "instruction.h"
#include "parser_code.h"

#define IS_OP(idx) (is->operands[(idx)].type != OP_NONE)

// Get text from operand on idx in is.
// If it already is a char * (e.g. label, variable name,...) the existing
// pointer is returned, If is not (e.g. a numeric value), returns a pointer to
// buf, in which it is now written. On any error returns empty string.
static const char *get_op_text(const struct Instruction_Statement *is,
                               size_t idx, char *buf, size_t bufsize);

void print_verbose(int condition, const char *string, ...) {
  if (!condition)
    return;

  printf("[VERBOSE] ");
  va_list args;
  va_start(args, string);
  vprintf(string, args);
  va_end(args);
}

void print_verbose_clean(int condition, const char *string, ...) {
  if (!condition)
    return;

  va_list args;
  va_start(args, string);
  vprintf(string, args);
  va_end(args);
}

void print_instruction(int condition, size_t line,
                       const struct Instruction_Statement *is, size_t addr) {
  char op1_buf[64] = "";
  char op2_buf[64] = "";
  const char *op1 = NULL;
  const char *op2 = NULL;
  if (!condition || !is || !is->descriptor || !is->descriptor->mnemonic)
    return;

  op1 = get_op_text(is, 0, op1_buf, sizeof(op1_buf));
  op2 = get_op_text(is, 1, op2_buf, sizeof(op2_buf));

  printf("[INSTR] L%zu: %s%s%s%s%s at CS:%zu\n", line, is->descriptor->mnemonic,
         IS_OP(0) ? " " : "", IS_OP(0) ? op1 : "", IS_OP(1) ? " " : "",
         IS_OP(1) ? op2 : "", addr);
}

static const char *get_op_text(const struct Instruction_Statement *is,
                               size_t idx, char *buf, size_t bufsize) {
  if (!is || idx >= 2)
    return "";

  const struct Operand *op = &is->operands[idx];
  switch (op->type) {
  case OP_NONE:
    return "";

  case OP_REG:
    return op->value.register_name;

  case OP_IMM32:
    switch (op->specifier) {
    case OPS_NONE:
      snprintf(buf, bufsize, "%d", op->value.immediate_value);
      return buf;
    case OPS_LABEL:
    case OPS_OFFSET:
      return op->value.label;
    default:
      return "";
    }

  default:
    return "";
  }
}
