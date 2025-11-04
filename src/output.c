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

  if (!fu_open(asp->config->target, &f, "wb")) {
    print_verbose(asp->config->flag_verbose, "Error opening a file.\n");
  }

  print_verbose(asp->config->flag_verbose, "Writing a KMA header'.\n");
  if (!fu_write_bytes(f, "KMA", 3)) {
    print_verbose(asp->config->flag_verbose,
                  "Couldn't write KMA at the beginning of the file.\n");
  }

  print_verbose(asp->config->flag_verbose, "Writing datasegment.\n");
  if (!fu_write_bytes(f, asp->dtsg->bytes, asp->dtsg->size)) {
    print_verbose(asp->config->flag_verbose,
                  "Couldn't write datasegment to the file.\n");
  }

  print_verbose(asp->config->flag_verbose, "Writing codesegment.\n");
  if (!fu_write_bytes(f, asp->cdsg->bytes, asp->cdsg->size)) {
    print_verbose(asp->config->flag_verbose,
                  "Couldn't write codesegment to the file.\n");
  }

  print_verbose(asp->config->flag_verbose, "Writing terminating NULL.\n");
  if (!fu_write_bytes(f, "\0", 1)) {
    print_verbose(asp->config->flag_verbose,
                  "Couldn't write NULL at the end of the file.\n");
  }

  print_verbose(asp->config->flag_verbose, "Closing file.\n");
  fclose(f);
  return ERR_NO_ERROR;
}
