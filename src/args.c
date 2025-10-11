#include "args.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_args(int argc, char **argv, struct Config *config) {
  size_t i, j;
  char *arg;
  char flag, unknown, verbose, instruction;
  if (argc < 2) {
    printf("Usage: ./kmas.exe <source.kas> [target.kmx] [-v] [-i]\n");
    return EXIT_FAILURE;
  }
  if (!argv || !config) {
    return EXIT_FAILURE;
  }

  config->source = argv[1];

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

  return EXIT_SUCCESS;
}
