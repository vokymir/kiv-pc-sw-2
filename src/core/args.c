#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "common.h"
#include "fileutil.h"
#include "memory.h"

// ===== MACRO HELPERS =====

#define RET_INVALID_INPUT_FILE(...)                                            \
  RET_STDERR(ERR_INVALID_INPUT_FILE, __VA_ARGS__)

#define RET_INVALID_OUTPUT_FILE(...)                                           \
  RET_STDERR(ERR_INVALID_OUTPUT_FILE, __VA_ARGS__)

// ===== PRIVATE FUNCTION DECLARATIONS =====

// Using ARGS find the source and return pointer to it.
// Return NULL on failure.
static const char *_args_find_src(const int argc, const char **argv);

// Using ARGS find the target and return pointer to it.
// Return NULL on failure.
static const char *_args_find_tgt(const int argc, const char **argv);

// Using ARGS find if verbose flag was set to true.
// Return 1 if was, 0 if wasnt.
static int _args_is_v(const int argc, const char **argv);

// Using ARGS find if instruction flag was set to true.
// Return 1 if was, 0 if wasnt.
static int _args_is_i(const int argc, const char **argv);

// Change extension from '.kas' to '.kmx'.
// Return 1 on success, 0 on failure.
static int _args_change_extension(char *path);

// ===== PARSING ARGS =====

enum Err_Main args_parse(struct Config *config, const int argc,
                         const char **argv) {
  const char *src = NULL, *tgt = NULL;
  int v = 0, i = 0, tgt_edit = 0;
  enum Err_Main err = ERR_NO_ERROR;

  if (argc < 2 || !argv || !config) { // Never could happen config == NULL
    printf("Usage: ./kmas.exe <source.kas> [target.kmx] [-v] [-i]\n");
    RET_INVALID_INPUT_FILE("The number of arguments was lesser than needed, or "
                           "function arguments were NULL.");
  }

  if (!(src = _args_find_src(argc, argv))) {
    RET_INVALID_INPUT_FILE("Couldn't find source path in CLI arguments.");
  }

  if (!(tgt = _args_find_tgt(argc, argv))) {
    tgt_edit = 1; // should edit the path in a moment
    tgt = src;    // same path for src and tgt file - if was not set
  }

  v = _args_is_v(argc, argv);
  i = _args_is_i(argc, argv);

  if (!args_config_init(config, src, tgt, v, i)) { // INIT CONFIG
    args_config_deinit(config);
    RET_INVALID_INPUT_FILE("Couldn't initialize config from given arguments.");
  }

  if (tgt_edit) { // target didnt exist, now must exit extension
    if (!_args_change_extension(config->target)) {
      args_config_deinit(config);
      // input because output is purely based on input file
      RET_INVALID_INPUT_FILE(
          "The target path was not given. When trying to use the source path, "
          "couldn't change the extension to .kmx.");
    }
  }

  // check both paths
  if (args_path_check_syntax(config->source, NULL, ".kas") != ARGS_NO_ERROR) {
    args_config_deinit(config);
    RET_INVALID_INPUT_FILE("The source path is not syntactically correct.");
  }

  if (args_path_check_syntax(config->target, NULL, ".kmx") != ARGS_NO_ERROR) {
    args_config_deinit(config);
    RET_INVALID_OUTPUT_FILE("The target path is not syntactically correct.");
  }

  if ((err = args_path_check_semantic(config)) != ERR_NO_ERROR) {
    RET_STDERR(err,
               "Either source or target path is not semantically correct.");
  }
  return ERR_NO_ERROR;
}

enum Err_Args args_path_check_syntax(const char *path, const char *prefix,
                                     const char *suffix) {
  size_t plen, len = 0;
  if (!path) {
    RET_STDERR(ARGS_INVALID_POINTER, "The given pointer to path is NULL.");
  }
  if ((plen = strlen(path)) == 0) {
    RET_STDERR(ARGS_PATH_EMPTY, "The length of given path is NULL.");
  }

  if (prefix) {
    len = strlen(prefix);
    if (plen < len || strncmp(path, prefix, len)) {
      RET_STDERR(ARGS_PATH_BAD_PREFIX,
                 "The given path '%s' doesn't have required prefix '%s'.", path,
                 prefix);
    }
  }

  if (suffix) {
    len = strlen(suffix);
    if (plen < len || strcmp((const char *)(path + plen - len), suffix)) {
      RET_STDERR(ARGS_PATH_BAD_EXTENSION,
                 "The given path '%s' doesn't have required suffix '%s'.", path,
                 suffix);
    }
  }

#if defined(_WIN32)
  if (strpbrk(path, "<>\"|?*") != NULL) {
    RET_STDERR(ARGS_PATH_SPECIAL_CHARS,
               "The given path contains invalid character(s).");
  }
#endif

  return ARGS_NO_ERROR;
}

enum Err_Main args_path_check_semantic(const struct Config *config) {
  if (!config || !config->source) {
    RET_INVALID_INPUT_FILE("Config or source is NULL.");
  } else if (!config->target) {
    RET_INVALID_OUTPUT_FILE("Target path is NULL.");
  }

  if (!fu_is_file(config->source)) { // source must exist and be file
    RET_INVALID_INPUT_FILE("Source path doesn't exist or is not a file.");
  }

  if (!fu_can_write(config->target)) {
    RET_INVALID_OUTPUT_FILE(
        "Target path must be a file or writable directory.");
  }

  return ERR_NO_ERROR;
}

// ===== WORKING w CONFIG =====

int args_config_init(struct Config *config, const char *source,
                     const char *target, int verbose, int instruction) {
  size_t len = 0;
  CLEANUP_IF_FAIL_ERR(config, "Config is NULL.");

  config->flag_instruction = instruction;
  config->flag_verbose = verbose;

  if (source) {
    len = strlen(source) + 1;
    config->source = jalloc(len);
    CLEANUP_IF_FAIL_ERR(config->source, "Source path is NULL.");
    strcpy(config->source, source);
  }

  if (target) {
    len = strlen(target) + 1;
    config->target = jalloc(len);
    CLEANUP_IF_FAIL_ERR(config->target, "Target path is NULL.");
    strcpy(config->target, target);
  }

  return 1;

cleanup:
  args_config_deinit(config);
  return 0;
}

void args_config_deinit(struct Config *config) {
  CLEANUP_IF_FAIL_ERR(config, "Config is NULL.");
  config->flag_verbose = 0;
  config->flag_instruction = 0;

  jree(config->source);
  config->source = NULL;
  jree(config->target);
  config->target = NULL;

cleanup:
  return;
}

// ===== PRIVATE FUNCTIONS =====

static const char *_args_find_src(const int argc, const char **argv) {
  CLEANUP_IF_FAIL_ERR(argc > 1 && argv && argv[1],
                      "Invalid arguments were given.");

  return argv[1];

cleanup:
  return NULL;
}

static const char *_args_find_tgt(const int argc, const char **argv) {
  int i = 0;
  CLEANUP_IF_FAIL_ERR(argc > 2 && argv, "Invalid arguments were given.");

  for (i = 2; i < argc; i++) { // skip .exe and src argumnets
    if (argv[i][0] != '-') {   // if is not a flag
      return argv[i];
    }
  }

cleanup:
  return NULL;
}

static int _args_is_v(const int argc, const char **argv) {
  int i = 0;
  CLEANUP_IF_FAIL_ERR(argc > 2 && argv, "Invalid arguments were given.");

  for (i = 2; i < argc; i++) { // skip .exe and src argumnets
    if (strcmp(argv[i], "-v") == 0) {
      return 1;
    }
  }

cleanup:
  return 0;
}

static int _args_is_i(const int argc, const char **argv) {
  int i = 0;
  CLEANUP_IF_FAIL_ERR(argc > 2 && argv, "Invalid arguments were given.");

  for (i = 2; i < argc; i++) { // skip .exe and src argumnets
    if (strcmp(argv[i], "-i") == 0) {
      return 1;
    }
  }

cleanup:
  return 0;
}

static int _args_change_extension(char *path) {
  char *begin = NULL;
  RET_STDERR_IF_FAIL(path, 0, "Path is NULL.");
  begin = strstr(path, ".kas"); // WARN: should maybe expect .kas only on the
                                // last four positions?
  RET_STDERR_IF_FAIL(begin, 0,
                     "The file on path doesn't have '.kas' extension.");
  *(char *)(begin + 2) = 'm'; // in .kas change a->m, s->x
  *(char *)(begin + 3) = 'x';
  return 1;
}
