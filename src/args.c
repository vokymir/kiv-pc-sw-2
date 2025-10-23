#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "common.h"
#include "fileutil.h"
#include "memory.h"

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

  if (argc < 2 || !argv || !config) { // Never could happen config == NULL
    printf("Usage: ./kmas.exe <source.kas> [target.kmx] [-v] [-i]\n");
    return ERR_INVALID_INPUT_FILE;
  }

  if (!(src = _args_find_src(argc, argv))) {
    return ERR_INVALID_INPUT_FILE;
  }

  if (!(tgt = _args_find_tgt(argc, argv))) {
    tgt_edit = 1; // should edit the path in a moment
    tgt = src;    // same path for src and tgt file - if was not set
  }

  v = _args_is_v(argc, argv);
  i = _args_is_i(argc, argv);

  if (!args_config_init(config, src, tgt, v, i)) { // INIT CONFIG
    args_config_deinit(config);
    return ERR_INVALID_INPUT_FILE;
  }

  if (tgt_edit) { // target didnt exist, now must exit extension
    if (!_args_change_extension(config->target)) {
      args_config_deinit(config);
      return ERR_INVALID_INPUT_FILE; // input because output is purely based on
                                     // input file
    }
  }

  // check both paths
  if (args_path_check_syntax(config->source, NULL, ".kas") != ARGS_NO_ERROR) {
    args_config_deinit(config);
    return ERR_INVALID_INPUT_FILE;
  }

  if (args_path_check_syntax(config->target, NULL, ".kmx") != ARGS_NO_ERROR) {
    args_config_deinit(config);
    return ERR_INVALID_OUTPUT_FILE;
  }

  // last and final check
  return args_path_check_semantic(config);
}

enum Err_Args args_path_check_syntax(const char *path, const char *prefix,
                                     const char *suffix) {
  size_t plen, len = 0;
  if (!path) {
    return ARGS_INVALID_POINTER;
  }
  if ((plen = strlen(path)) == 0) {
    return ARGS_PATH_EMPTY;
  }

  if (prefix) {
    len = strlen(prefix);
    if (plen < len || strncmp(path, prefix, len)) {
      return ARGS_PATH_BAD_PREFIX;
    }
  }

  if (suffix) {
    len = strlen(suffix);
    if (plen < len || strcmp((const char *)(path + plen - len), suffix)) {
      return ARGS_PATH_BAD_EXTENSION;
    }
  }

#if defined(_WIN32)
  if (strpbrk(path, "<>\"|?*") != NULL) {
    return ARGS_PATH_SPECIAL_CHARS;
  }
#endif

  return ARGS_NO_ERROR;
}

enum Err_Main args_path_check_semantic(const struct Config *config) {
  if (!config ||
      !config
           ->source) { // this shouldn't happen, so suspect first possible error
    return ERR_INVALID_INPUT_FILE;
  } else if (!config->target) {
    return ERR_INVALID_OUTPUT_FILE;
  }

  if (!fu_is_file(config->source)) { // source must exist and be file
    return ERR_INVALID_INPUT_FILE;
  }

  if (!fu_can_write(
          config->target)) { // target must exist or be in writable directory
    return ERR_INVALID_OUTPUT_FILE;
  }

  return ERR_NO_ERROR;
}

// ===== WORKING w CONFIG =====

int args_config_init(struct Config *config, const char *source,
                     const char *target, int verbose, int instruction) {
  size_t len = 0;
  CLEANUP_IF_FAIL(config);

  config->flag_instruction = instruction;
  config->flag_verbose = verbose;

  if (source) {
    len = strlen(source) + 1;
    config->source = jalloc(len);
    CLEANUP_IF_FAIL(config->source);
    strcpy(config->source, source);
  }

  if (target) {
    len = strlen(target) + 1;
    config->target = jalloc(len);
    CLEANUP_IF_FAIL(config->target);
    strcpy(config->target, target);
  }

  return 1;

cleanup:
  args_config_deinit(config);
  return 0;
}

void args_config_deinit(struct Config *config) {
  CLEANUP_IF_FAIL(config);
  config->flag_verbose = 0;
  config->flag_instruction = 0;

  jree_clear((void **)&config->source);
  jree_clear((void **)&config->target);

cleanup:
  return;
}

// ===== PRIVATE FUNCTIONS =====

static const char *_args_find_src(const int argc, const char **argv) {
  CLEANUP_IF_FAIL(argc > 1 && argv && argv[1]);

  return argv[1];

cleanup:
  return NULL;
}

static const char *_args_find_tgt(const int argc, const char **argv) {
  int i = 0;
  CLEANUP_IF_FAIL(argc > 2 && argv);

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
  CLEANUP_IF_FAIL(argc > 2 && argv);

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
  CLEANUP_IF_FAIL(argc > 2 && argv);

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
  if (!path) {
    return 0;
  }
  begin = strstr(path, ".kas");
  if (!begin) {
    return 0;
  }
  *(char *)(begin + 2) = 'm'; // in .kas change a->m, s->x
  *(char *)(begin + 3) = 'x';
  return 1;
}
