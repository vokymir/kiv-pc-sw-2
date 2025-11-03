#include <stdio.h>

#include "common.h"
#include "fileutil.h"
#include "output.h"

enum Err_Main output_binary(const struct Assembler_Processing *asp) {
  FILE *f = NULL;
  if (!asp || !asp->config || !asp->cdsg || !asp->dtsg ||
      !asp->config->target) {
    print_verbose(
        asp && asp->config && asp->config->flag_verbose,
        "Tried to write output binary, but some argument is missing.\n");
    return ERR_MY_CODE_FAILURE;
  }

  print_verbose(asp->config->flag_verbose, "Opening a file '%s'.\n",
                asp->config->target);

  if (!fu_open(asp->config->target, &f, "rb")) {
    print_verbose(asp->config->flag_verbose, "Error opening a file.\n");
  }

  fu_write_bytes(f, "KMA", 3);

  fclose(f);
  return ERR_NO_ERROR;
}
