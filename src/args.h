#ifndef ARGS_H
#define ARGS_H

#include "common.h"

// Errors specific for ARGumentS module.
enum Err_Args {
  ARGS_NO_ERROR = 0,
  ARGS_INVALID_POINTER = 1,
  ARGS_PATH_EMPTY = 2,
  ARGS_PATH_BAD_EXTENSION = 3,
  ARGS_PATH_SPECIAL_CHARS = 4,
};

// Parse all arguments given and write the results into
//  the given Config structure. Returns adequate Err_Main
enum Err_Main args_parse(int argc, char **argv, struct Config *config);

enum Err_Args args_path_syntax_check(char *path);

// Free target in config.
void args_free_config(struct Config *config);

#endif
