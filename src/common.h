#ifndef COMMON_H
#define COMMON_H

// Holds information needed throughout the whole program.
struct Config {
  char flag_verbose;
  char flag_instruction;
  char *source;
  char *target;
};

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

#endif
