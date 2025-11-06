#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "fileutil.h"
#include "output.h"

#define PRINT_VERBOSE(...) print_verbose(asp->config->flag_verbose, __VA_ARGS__)

#define CLEANUP_VERBOSE_IF_FAIL(cond, ...)                                     \
  do {                                                                         \
    if (!(cond)) {                                                             \
      PRINT_VERBOSE(__VA_ARGS__);                                              \
      err = ERR_MY_CODE_FAILURE;                                               \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

// Write little endian 32bit number in little endian.
// Return 1 on success, 0 on failure.
int _write_in_little_endian(FILE *f, size_t number);

enum Err_Main output_binary(const struct Assembler_Processing *asp) {
  FILE *f = NULL;
  enum Err_Main err = ERR_NO_ERROR;

  if (!asp || !asp->config || !asp->cdsg || !asp->dtsg ||
      !asp->config->target) {
    PRINT_ERR_1ST_NULL_ARG(asp, asp->config, asp->cdsg, asp->dtsg,
                           asp->config->target);
    PRINT_VERBOSE(
        "Tried to write output binary, but some argument is missing.\n");
    return ERR_MY_CODE_FAILURE;
  }

  PRINT_VERBOSE("Opening a file '%s'.\n", asp->config->target);

  CLEANUP_VERBOSE_IF_FAIL(fu_open(asp->config->target, &f, "wb"),
                          "Error opening file '%s'.\n", asp->config->target);

  PRINT_VERBOSE("Writing a KMX header.\n");
  CLEANUP_VERBOSE_IF_FAIL(fu_write_bytes(f, KMX_SIGNATURE, KMX_SIGNATURE_LEN),
                          "Couldn't write KMA at the beginning of the file.\n");

  PRINT_VERBOSE("Writing length of data segment.\n");
  CLEANUP_VERBOSE_IF_FAIL(asp->dtsg->size <= INT32_MAX,
                          "The size of datasegment is too big.");
  CLEANUP_VERBOSE_IF_FAIL(_write_in_little_endian(f, asp->dtsg->size),
                          "Couldn't write NULL terminator.\n");

  PRINT_VERBOSE("Writing datasegment.\n");
  CLEANUP_VERBOSE_IF_FAIL(fu_write_bytes(f, asp->dtsg->bytes, asp->dtsg->size),
                          "Couldn't write datasegment to the file.\n");

  PRINT_VERBOSE("Writing codesegment.\n");
  CLEANUP_VERBOSE_IF_FAIL(fu_write_bytes(f, asp->cdsg->bytes, asp->cdsg->size),
                          "Couldn't write codesegment to the file.\n");

  PRINT_VERBOSE("Writing terminating NULL.\n");
  CLEANUP_VERBOSE_IF_FAIL(fu_write_bytes(f, "\0", 1),
                          "Couldn't write NULL terminator.\n");

cleanup:
  if (f) {
    PRINT_VERBOSE("Closing file.\n");
    fclose(f);
  }
  return err;
}

int _write_in_little_endian(FILE *f, size_t number) {
  int32_t n;
  uint8_t bytes[KMA_DTSG_LEN_SIZE];
  size_t i;
  RET_STDERR_IF_FAIL(f, 0, "Tried to write to NULL file.");

  if (number > INT32_MAX) {
    return 0;
  }
  n = (int32_t)number;

  // convert to little endian
  for (i = 0; i < KMA_DTSG_LEN_SIZE; i++) {
    bytes[i] = (uint8_t)((n >> (i * 8)) & 0xFF);
  }

  return fu_write_bytes(f, &bytes, KMA_DTSG_LEN_SIZE);
}
