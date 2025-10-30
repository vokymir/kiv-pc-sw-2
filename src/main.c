#include <assert.h>
#include <stdio.h>

#include "args.h"
#include "assembler.h"
#include "common.h"
#include "memory.h"
#include "output.h"

#define DONT_FAIL(func)                                                        \
  do {                                                                         \
    if ((err = (func)) != ERR_NO_ERROR) {                                      \
      goto finalize;                                                           \
    }                                                                          \
  } while (0)

int main(const int argc, const char **argv) {
  struct Config config = {0};
  struct Assembler_Processing *asp = NULL;
  enum Err_Main err = ERR_NO_ERROR;

  // Parse arguments and save results into config.
  DONT_FAIL(args_parse(&config, argc, argv));

  printf("Source: %s\nTarget: %s\nVerbose: %s\nInstructions: %s\n",
         config.source, config.target, config.flag_verbose ? "yes" : "no",
         config.flag_instruction ? "yes" : "no");

  asp = asp_create(&config, NULL, NULL, NULL);
  if (!asp) {
    return -1; // placeholder
  }

  DONT_FAIL(process_assembler(asp));

  DONT_FAIL(output_binary(asp));

// Free all main-related memory, check for leaks and end
finalize:
  args_config_deinit(&config);
  assert(jemory() == 0);
  return err;
}
