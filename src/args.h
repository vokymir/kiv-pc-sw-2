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
  ARGS_PATH_BAD_PREFIX = 5,
};

// ===== PARSING/VALIDATING ARGS =====

// Parse all arguments given and write the results into
// the given Config structure. Assumes the first argument is the name of running
// executable. Performs static syntax and semantic check on source/target by
// calling args_path_check_syntax/semantic. Returns adequate Err_Main
enum Err_Main args_parse(struct Config *config, const int argc,
                         const char **argv);

// Perform static syntax check on any path, checking if it even could be a path.
// Checking prefix/suffix is omitted on empty string or NULL but it means, that
// the path must have that prefix or suffix. Use prefix for e.g. ensuring some
// parent folder, suffix for e.g. file extension. Return adequate error code.
enum Err_Args args_path_check_syntax(const char *path, const char *prefix,
                                     const char *suffix);

// Perform semantic check on all paths inside config.
// Source path must exist and be file. Target path doesn't have to exist, but at
// least the parent directory must allows writing, so the target file can be
// created later.
enum Err_Main args_path_check_semantic(const struct Config *config);

// ===== WORKING w CONFIG =====

// Initialize all config insides, if paths are given, copies them. These must be
// freed using args_config_deinit. Return 1 on success, 0 on failure.
int args_config_init(struct Config *config, const char *source,
                     const char *target, int verbose, int instruction);

// Set all config members to 0 and free all insides.
void args_config_deinit(struct Config *config);

#endif
