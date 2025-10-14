#include <assert.h>
#include <stdio.h>

#include "args.h"
#include "common.h"
#include "memory.h"

int main(const int argc, const char **argv) {
  struct Config config = {0};
  enum Err_Main err = ERR_NO_ERROR;

  // Parse arguments and save results into config.
  if ((err = args_parse(argc, argv, &config)) != ERR_NO_ERROR) {
    goto finalize;
  }

  // Check source/target paths.
  if ((err = args_check_config(&config)) != ERR_NO_ERROR) {
    goto finalize;
  }

  printf("Source: %s\nTarget: %s\nVerbose: %s\nInstructions: %s\n",
         config.source, config.target, config.flag_verbose ? "yes" : "no",
         config.flag_instruction ? "yes" : "no");

  // Free all main-related memory, check for leaks and end
finalize:
  args_config_free(&config);
  assert(jemory() == 0);
  return err;
}
