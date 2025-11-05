#ifndef ASSEMBLER_INTERNAL_H
#define ASSEMBLER_INTERNAL_H

#include "common.h"

// If condition fail, set variable 'err' to given er
// & goto cleanup.
#define ERR_IF_FAIL(cond, er)                                                  \
  do {                                                                         \
    if (!(cond)) {                                                             \
      err = (er);                                                              \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

// Check if func == ASM_NO_ERROR
// Reuse error given from func & goto cleanup
#define REUSE_ERR_IF_FAIL(func)                                                \
  do {                                                                         \
    if ((err = (func)) != ASM_NO_ERROR) {                                      \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

#define PRINT_VERBOSE(...)                                                     \
  print_verbose(asp && asp->config && asp->config->flag_verbose, __VA_ARGS__)
#define PRINT_VERBOSE_CLN(...)                                                 \
  print_verbose_clean(asp && asp->config && asp->config->flag_verbose,         \
                      __VA_ARGS__)
#define PRINT_VERBOSE_DBG(...)                                                 \
  print_verbose(DEBUG, __VA_ARGS__) // TODO: Is DEBUG used? remove

// If condition fail, print verbose clean & return err.
#define RET_VERBOSE_CLN_IF_FAIL(cond, err, ...)                                \
  do {                                                                         \
    if (!(cond)) {                                                             \
      PRINT_VERBOSE_CLN(__VA_ARGS__);                                          \
      return err;                                                              \
    }                                                                          \
  } while (0)

#define IF_FAIL(cond) if (!(cond))

#endif
