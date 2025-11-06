#include <assert.h>

#include "args.h"
#include "assembler.h"
#include "common.h"
#include "memory.h"
#include "output.h"

// If any called function fails, main won't fail, won't leak any memory.
#define DONT_FAIL(func)                                                        \
  do {                                                                         \
    if ((err = (func)) != ERR_NO_ERROR) {                                      \
      goto finalize;                                                           \
    }                                                                          \
  } while (0)

// Run the transcriber program: With arguments from CLI translate given
// assembler file into binary file for the virtual KMA computer.
// Return codes were specified in assignment, and are defined in common.h
int main(const int argc, const char **argv) {
  struct Config config = {0};
  struct Assembler_Processing *asp = NULL;
  enum Err_Main err = ERR_NO_ERROR;

  // Parse CLI arguments and save results into config.
  DONT_FAIL(args_parse(&config, argc, argv));

  // Prepare for processing assembler
  asp = asp_create(&config, NULL, NULL, NULL);
  if (!asp) {
    return ERR_MY_CODE_FAILURE;
  }

  // Store the intermediate representation logically separated in asp.
  DONT_FAIL(process_assembler(asp));

  // Assemble the binary and save it to output file.
  DONT_FAIL(output_binary(asp));

// Free all main-related memory.
finalize:
  args_config_deinit(&config);
  if (asp) {
    asp_free(&asp);
  }
  assert(jemory() == 0); // Sanity check
  return (int)err;
}
