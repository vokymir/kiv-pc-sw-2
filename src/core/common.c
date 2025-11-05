#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common.h"
#include "instruction.h"
#include "parser.h"

#define IS_OP(idx) (is->operands[(idx)].type != OP_NONE)

// Get text from operand on idx in is.
// If it already is a char * (e.g. label, variable name,...) the existing
// pointer is returned, If is not (e.g. a numeric value), returns a pointer to
// buf, in which it is now written. On any error returns empty string.
static const char *_get_op_text(const struct Instruction_Statement *is,
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
  if (!condition) {
    return;
  }
  if (!is || !is->descriptor || !is->descriptor->mnemonic) {
    PRINT_ERR("Instruction statement is invalid.");
    return;
  }

  op1 = _get_op_text(is, 0, op1_buf, sizeof(op1_buf));
  op2 = _get_op_text(is, 1, op2_buf, sizeof(op2_buf));

  printf("[INSTR] L%zu: %s%s%s%s%s at CS:%zu\n", line, is->descriptor->mnemonic,
         IS_OP(0) ? " " : "", IS_OP(0) ? op1 : "", IS_OP(1) ? " " : "",
         IS_OP(1) ? op2 : "", addr);
}

void print_err(const char *filename, size_t line, const char *string, ...) {
  size_t len = 0;
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  char timebuf[32];
  if (!string) {
    return;
  }

  if (t) {
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", t);
  } else {
    snprintf(timebuf, sizeof(timebuf), "unknown-time");
  }

  fprintf(stderr, "[%s] {%s:%zu} ", timebuf,
          filename ? filename : "unspecified", line);

  va_list args;
  va_start(args, string);
  vfprintf(stderr, string, args);
  va_end(args);

  // ensure newline at end
  len = strlen(string);
  if (len == 0 || string[len - 1] != '\n') {
    fputc('\n', stderr);
  }
}

size_t first_null_arg(const void *args[], size_t count) {
  size_t i = 0;
  for (i = 0; i < count; i++) {
    if (args[i] == NULL) {
      return i + 1;
    }
  }
  return 0;
}

int valid_args(size_t count, ...) {
  size_t i = 0;
  void *ptr = NULL;

  va_list args;
  va_start(args, count);

  for (i = 0; i < count; i++) {
    ptr = va_arg(args, void *);
    if (ptr == NULL) {
      va_end(args);
      return 0;
    }
    ptr = NULL;
  }

  va_end(args);
  return 1;
}

static const char *_get_op_text(const struct Instruction_Statement *is,
                                size_t idx, char *buf, size_t bufsize) {
  if (!is || idx >= KMA_MAX_OPERANDS) {
    PRINT_ERR("Invalid arguments.");
    return "";
  }

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
    PRINT_ERR("Unknown operand type.");
    return "";
  }
}
