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
  ARGS_PATH_BAD_PREFIX = 5,    // only for completeness
};

// ===== PARSING/VALIDATING ARGS =====

// Parse all arguments given and write the results into
// the given Config structure. Performs static syntax check
// on source/target by calling args_path_syntax_check.
// Returns adequate Err_Main
enum Err_Main args_parse(const int argc, const char **argv,
                         struct Config *config);

// Perform static syntax check on any path.
// Checking prefix/suffix is omitted on empty string or NULL
// Use suffix e.g. for file extension.
enum Err_Args args_path_syntax_check(const char *path, const char *prefix,
                                     const char *suffix);

// Perform semantic check on source/target paths in config.
// Return TODO: should be Err_Main or Err_Args?
enum Err_Main args_check_config(const struct Config *config);

// ===== WORKING w CONFIG =====

// Clear all config members to 0.
// Danger: if target was initialized, it is freed here.
void args_config_clear(struct Config *config);

// Initialize target path in config;
// Meaning it copies from target, allocating new memory.
// Return 1 on success, 0 on failure.
int args_config_init(struct Config *config, const char *target);

// Free target in config.
// This is responsibility of Main, contrary to its name.
// Config is inited in Args, but should only be freed at the end of program,
// which is at the end of Main.
void args_config_free(struct Config *config);

// ===== (private) HELPERS =====

// Parse one argument. Set flags or target path in config.
// Return ERR_NO_ERROR on success.
enum Err_Main _args_parse_arg(const char *arg, struct Config *config);

// Change extension from '.kas' to '.kmx'.
// Return 1 on success, 0 on failure.
int _args_change_extension(char *path);

#endif
