#include <assert.h>
#include <stdio.h>

#include "args.h"
#include "common.h"
#include "fileutil.h"
#include "memory.h"

int main(const int argc, const char **argv) {
  struct Config config;
  enum Err_Main err = ERR_NO_ERROR;

  if ((err = args_parse(argc, argv, &config)) != ERR_NO_ERROR) {
    goto finalize;
  }

  if (!fu_is_file(config.source)) {
    err = ERR_INVALID_INPUT_FILE;
    goto finalize;
  }

  printf("Source: %s\nTarget: %s\nVerbose: %s\nInstructions: %s\n",
         config.source, config.target, config.flag_verbose ? "yes" : "no",
         config.flag_instruction ? "yes" : "no");

finalize:
  args_config_free(&config);
  assert(jemory() == 0);
  return err;
}
