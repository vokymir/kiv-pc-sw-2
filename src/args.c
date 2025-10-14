#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "common.h"
#include "memory.h"

enum Err_Main args_parse(const int argc, const char **argv,
                         struct Config *config) {
  size_t i;
  enum Err_Main merr;
  enum Err_Args err;
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
  size_t plen, len;
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

#if defined(_WIN32) || defined(_WIN64)
  if (strpbrk(path, "<>\"|?*") != NULL) {
    return ARGS_PATH_SPECIAL_CHARS;
  }
#endif

  return ARGS_NO_ERROR;
}

void args_config_clear(struct Config *config) {
  if (!config) {
    return;
  }
  config->flag_verbose = 0;
  config->flag_instruction = 0;
  config->source = NULL;
  config->target = NULL;
}

int args_config_init(struct Config *config, const char *target) {
  size_t len;
  if (!config) {
    return 1;
  }

  len = strlen(target) + 1;
  config->target = jalloc(len);
  if (!config->target) {
    return 1;
  }
  strcpy(config->target, target);

  return 0;
}

void args_config_free(struct Config *config) {
  if (!config || !config->target)
    return;
  jree_clear((void **)&config->target); // free & clear the config->target
}

enum Err_Main _args_parse_arg(const char *arg, struct Config *config) {
  int res;
  if (strcmp(arg, "-v") == 0) { // is verbose?
    config->flag_verbose = 1;
  } else if (strcmp(arg, "-i") == 0) { // Is instruction?
    config->flag_instruction = 1;
  } else if (!config->target) { // Is target still empty?
    res = args_config_init(config, arg);
    if (res) { // Was filling target a failure?
      return ERR_INVALID_OUTPUT_FILE;
    }
  }
  return ERR_NO_ERROR;
}

int _args_change_extension(const char *path) {
  char *begin;
  if (!path) {
    return 1;
  }
  begin = strstr(path, ".kas");
  if (!begin) {
    return 1;
  }
  *(char *)(begin + 2) = 'm'; // in .kas change a->m, s->x
  *(char *)(begin + 3) = 'x';
  return 0;
}
