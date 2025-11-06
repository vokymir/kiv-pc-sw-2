#ifndef ARGS_H
#define ARGS_H

#include <stddef.h>

#include "common.h"

// Errors specific for ARGumentS module.
enum Err_Args {
  ARGS_NO_ERROR = 0,           // No error happened
  ARGS_INVALID_POINTER = 1,    // Pointer given as argument is invalid
  ARGS_PATH_EMPTY = 2,         // Given path is an empty string
  ARGS_PATH_BAD_EXTENSION = 3, // Path had invalid extension
                               // (used on .kas & .kmx)
  ARGS_PATH_SPECIAL_CHARS = 4, // only on Windows, don't allow specific chars
  ARGS_PATH_BAD_PREFIX = 5,    // Given path is in invalid location
};

// ===== PARSING/VALIDATING ARGS =====

// Parse all arguments given and write the results into the given Config
// structure. Assumes the first argument is the name of running executable
// (specified in assignment). Performs static syntax and semantic check on
// source and target by calling args_path_check_syntax/semantic. Returns
// corresponding Err_Main
enum Err_Main args_parse(struct Config *config, const int argc,
                         const char **argv);

// Perform static syntax check on any path, checking if it even could be a path,
// without actually working with the filesystem or opening any file.  The path
// must have that prefix or suffix. Checking prefix/suffix is omitted on empty
// string or NULL. Use prefix for e.g. ensuring some parent folder, suffix for
// e.g. file extension. Return corresponding error code.
enum Err_Args args_path_check_syntax(const char *path, const char *prefix,
                                     const char *suffix);

// Perform semantic check on all paths inside config. Uses filesystem api via
// standard library. Source path must exist and be file. Target path doesn't
// have to exist, but at least the parent directory must allows writing, so the
// target file can be created later.
enum Err_Main args_path_check_semantic(const struct Config *config);

// ===== WORKING w CONFIG =====
// in the code was never dynamically allocated, so no create/free

// Initialize all config insides, if paths are given, copies them. These must be
// freed using args_config_deinit. Return 1 on success, 0 on failure.
int args_config_init(struct Config *config, const char *source,
                     const char *target, int verbose, int instruction);

// Free all insides of config and set NULL pointers.
void args_config_deinit(struct Config *config);

#endif
