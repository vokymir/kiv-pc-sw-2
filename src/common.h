#ifndef COMMON_H
#define COMMON_H

// Implementing RAII-like cleanup procedure.
// If condition is not satisfied, go to label.
// Label must be at the end of the same function, to work reliably.
#include "parser_code.h"
#define GOTO_IF_FAIL(cond, label)                                              \
  do {                                                                         \
    if (!(cond))                                                               \
      goto label;                                                              \
  } while (0)

// If condition is not satisfied, go to label 'cleanup'.
#define CLEANUP_IF_FAIL(cond) GOTO_IF_FAIL((cond), cleanup)

#define RETURN_IF_FAIL(cond, retval)                                           \
  do {                                                                         \
    if (!(cond))                                                               \
      return retval;                                                           \
  } while (0)

// Errors specific to main, which the program outputs.
enum Err_Main {
  ERR_NO_ERROR = 0,
  ERR_INVALID_INPUT_FILE = 1,
  ERR_INVALID_OUTPUT_FILE = 2,
  ERR_SYNTAX_ERROR = 3,
  ERR_FILE_ACCESS_FAILURE = 4,
  ERR_OUT_OF_MEMORY = 5,
  ERR_UNRESOLVED_REFERENCE = 6,
  ERR_CODE_SEGMENT_TOO_LARGE = 7,
  ERR_DATA_SEGMENT_TOO_LARGE = 8,
};

// Holds information needed throughout the whole program.
struct Config {
  int flag_verbose;
  int flag_instruction;
  char *source;
  char *target;
};

// United verbose output to console.
// Only print if condition is met, variadic arguments will be forwarded to
// printf function from stdio.h, which require the first argument to be
// *string*, the next anything 'mentioned' in the string.
void print_verbose(int condition, const char *string, ...);

// United instruction output to console.
// Only print if condition is met.
// Format:
// L<line>: <is.name> [<is.operands>] at CS:<addr>
// e.g.:
// L50: DEC A at CS:123
void print_instruction(int condition, size_t line,
                       struct Instruction_Statement *is, size_t addr);

#endif
