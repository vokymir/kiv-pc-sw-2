#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"
#include "common.h"
#include "memory.h"

enum Err_Main args_parse(const int argc, const char **argv,
                         struct Config *config) {
  size_t i, j;
  char *arg;
  enum Err_Args err;
  if (argc < 2 || !argv || !config) { // Never could happen config == NULL
    printf("Usage: ./kmas.exe <source.kas> [target.kmx] [-v] [-i]\n");
    return ERR_INVALID_INPUT_FILE;
  }

  args_config_clear(config);        // ensure config is clear
  config->source = (char *)argv[1]; // source save and validation
  if ((err = args_path_syntax_check(config->source) != ARGS_NO_ERROR)) {
    return ERR_INVALID_INPUT_FILE;
  }

  for (i = 2; i < (size_t)argc; ++i) {
    arg = argv[i];
    if (strcmp(arg, "-v")) {
      config->flag_verbose = 1;
    } else if (strcmp(arg, "-i")) {
      config->flag_instruction = 1;
    } else {
      args_config_init(config, arg);
    }
  }

  return ERR_NO_ERROR;
}

enum Err_Args args_path_syntax_check(const char *path) {
  size_t len;
  if (!path) {
    return ARGS_INVALID_POINTER;
  }
  if ((len = strlen(path)) == 0) {
    return ARGS_PATH_EMPTY;
  }
  if (strcmp((char *)(path + len - 4), ".kas")) {
    return ARGS_PATH_BAD_EXTENSION;
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
  jree(config->target);
}
