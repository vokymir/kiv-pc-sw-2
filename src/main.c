#include <stdio.h>
#include <stdlib.h>

#include "args.h"
#include "common.h"

int main(int argc, char **argv) {
  struct Config config;
  enum Err_Main err;

  if ((err = args_parse(argc, argv, &config)) != ERR_NO_ERROR) {
    return err;
  }

  printf("Source: %s\nTarget: %s\nVerbose: %s\nInstructions: %s\n",
         config.source, config.target, config.flag_verbose ? "yes" : "no",
         config.flag_instruction ? "yes" : "no");

  return EXIT_SUCCESS;
}
