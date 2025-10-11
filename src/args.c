#include "args.h"
#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum Err_Main args_parse(int argc, char **argv, struct Config *config) {
  size_t i, j;
  char *arg;
  char flag, unknown, verbose, instruction;
  enum Err_Args err;
  if (argc < 2 || !argv || !config) { // Never could happen config == NULL
    printf("Usage: ./kmas.exe <source.kas> [target.kmx] [-v] [-i]\n");
    return ERR_INVALID_INPUT_FILE;
  }

  config->source = argv[1];

  if ((err = args_path_syntax_check(config->source) != ARGS_NO_ERROR)) {
    return ERR_INVALID_INPUT_FILE;
  }

  flag = '-';
  verbose = 'v';
  instruction = 'i';
  for (i = 2; i < (size_t)argc; ++i) {
    arg = argv[i];
    unknown = arg[0];
    if (flag == unknown) {
      for (j = 1; j < strlen(arg); ++j) {
        unknown = arg[j];
        if (unknown == verbose) {
          config->flag_verbose = 1;
        } else if (unknown == instruction) {
          config->flag_instruction = 1;
        }
      }
    }
  }

  return ERR_NO_ERROR;
}

enum Err_Args args_path_syntax_check(char *path) {
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

void args_free_config(struct Config *config) {
  if (!config || !config->target)
    return;
  free(config->target);
}
