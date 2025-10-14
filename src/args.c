#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "common.h"
#include "fileutil.h"
#include "memory.h"

// ===== PARSING ARGS =====

enum Err_Main args_parse(const int argc, const char **argv,
                         struct Config *config) {
  size_t i = 0;
  enum Err_Main merr = ERR_NO_ERROR;
  enum Err_Args err = ARGS_NO_ERROR;
  if (argc < 2 || !argv || !config) { // Never could happen config == NULL
    printf("Usage: ./kmas.exe <source.kas> [target.kmx] [-v] [-i]\n");
    return ERR_INVALID_INPUT_FILE;
  }

  args_config_clear(config); // ensure config is clear
  config->source = argv[1];  // source save and validation
  if ((err = args_path_syntax_check(config->source, NULL, ".kas")) !=
      ARGS_NO_ERROR) {
    return ERR_INVALID_INPUT_FILE;
  }

  for (i = 2; i < (size_t)argc; ++i) { // parse all arguments
    if ((merr = _args_parse_arg(argv[i], config)) != ERR_NO_ERROR) {
      return merr;
    }
  }

  if (!config->target) { // if no target path, set default
    if (!args_config_init(config,
                          config->source)) { // set to the same as source
      return ERR_INVALID_OUTPUT_FILE;
    }
    if (!_args_change_extension(config->target)) { // only edit extension
      return ERR_INVALID_OUTPUT_FILE;
    }
  }

  return ERR_NO_ERROR;
}

enum Err_Args args_path_syntax_check(const char *path, const char *prefix,
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

enum Err_Main args_check_config(const struct Config *config) {
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

void args_config_clear(struct Config *config) {
  if (!config) {
    return;
  }
  config->flag_verbose = 0;
  config->flag_instruction = 0;
  config->source = NULL;
  if (config->target) {
    args_config_free(config);
  }
  config->target = NULL;
}

int args_config_init(struct Config *config, const char *target) {
  size_t len = 0;
  if (!config) {
    return 0;
  }

  len = strlen(target) + 1;
  config->target = jalloc(len);
  if (!config->target) {
    return 0;
  }
  if (!strcpy(config->target, target)) {
    return 0;
  }

  return 1;
}

void args_config_free(struct Config *config) {
  if (!config || !config->target) {
    return;
  }
  jree_clear((void **)&config->target); // free & clear the config->target
}

// ===== (private) HELPERS =====

enum Err_Main _args_parse_arg(const char *arg, struct Config *config) {
  if (strcmp(arg, "-v") == 0) { // is verbose?
    config->flag_verbose = 1;
  } else if (strcmp(arg, "-i") == 0) { // Is instruction?
    config->flag_instruction = 1;
  } else if (!config->target) {           // Is target still empty?
    if (!args_config_init(config, arg)) { // Was filling target a failure?
      return ERR_INVALID_OUTPUT_FILE;
    }
  }
  return ERR_NO_ERROR;
}

int _args_change_extension(char *path) {
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
