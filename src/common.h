#ifndef COMMON_H
#define COMMON_H

// Implementing RAII-like cleanup procedure.
// If condition is not satisfied, go to label.
// Label must be at the end of the same function, to work reliably.
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

#endif
