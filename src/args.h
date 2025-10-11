#ifndef ARGS_H
#define ARGS_H

#include "common.h"

// Parse all arguments given and write the results into
//  the given Config structure. Returns EXIT_SUCCESS/FAILURE.
enum Err_Main args_parse(int argc, char **argv, struct Config *config);

enum Err_Main args_path_syntax_check(char *path);

// Free target in config.
void args_free_config(struct Config *config);

// Errors specific for ARGumentS module.
enum Err_Args {
  ARGS_NO_ERROR = 0,
  ARGS_INVALID_POINTER = 1,
  ARGS_EMPTY_PATH = 2,
  ARGS_BAD_EXTENSION = 3,
};

#endif
