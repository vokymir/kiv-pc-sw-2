#ifndef ARGS_H
#define ARGS_H

#include <stddef.h>

#include "common.h"

// Errors specific for ARGumentS module.
enum Err_Args {
  ARGS_NO_ERROR = 0,
  ARGS_INVALID_POINTER = 1,
  ARGS_PATH_EMPTY = 2,
  ARGS_PATH_BAD_EXTENSION = 3,
  ARGS_PATH_SPECIAL_CHARS = 4, // only on Windows
};

// Parse all arguments given and write the results into
// the given Config structure. Performs static syntax check
// on source/target by calling args_path_syntax_check.
// Returns adequate Err_Main
enum Err_Main args_parse(const int argc, const char **argv,
                         struct Config *config);

// Perform static syntax check on any path.
enum Err_Args args_path_syntax_check(const char *path);

// Clear all config members to 0.
// Danger: Don't call after init, or any allocation.
void args_config_clear(struct Config *config);

// Initialize target path in config;
// Return 0 on success, 1 on failure.
int args_config_init(struct Config *config, const char *target);

// Free target in config.
void args_config_free(struct Config *config);

#endif
