#ifndef ASSEMBLER_INTERNAL_H
#define ASSEMBLER_INTERNAL_H

#include "common.h"
#include <stddef.h>

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

// If condition fail, print verbose clean & return err.
#define RET_VERBOSE_CLN_IF_FAIL(cond, err, ...)                                \
  do {                                                                         \
    if (!(cond)) {                                                             \
      PRINT_VERBOSE_CLN(__VA_ARGS__);                                          \
      return (err);                                                            \
    }                                                                          \
  } while (0)

#define IF_FAIL(cond) if (!(cond))

// guard against invalid arguments in pass1 / pass2
// checks if 'cond' is FALSE, if so, print standard verbose & stderr output and
// return err
#define INVALID_ARGS_PRINTS_ERR_IF_FAIL(cond)                                  \
  do {                                                                         \
    IF_FAIL((cond)) {                                                          \
      PRINT_VERBOSE_CLN("but something went WRONG.");                          \
      PRINT_ERR("Invalid argument.");                                          \
      return ASM_INVALID_ARGS;                                                 \
    }                                                                          \
  } while (0)

// guard against invalid arguments in pass1 / pass2
// checks if any argument (given as variadic) is NULL, if so, print standard
// verbose & stderr output and return ASM_INVALID_ARGS
#define RET_PRINT_ERR_IF_FAIL_ARGS(...)                                        \
  do {                                                                         \
    if (!(VALID_ARGS(__VA_ARGS__))) {                                          \
      PRINT_VERBOSE_CLN("but something went WRONG.");                          \
      PRINT_ERR_1ST_NULL_ARG(__VA_ARGS__);                                     \
      return ASM_INVALID_ARGS;                                                 \
    }                                                                          \
  } while (0)

#define RET_PRINT_ERR_IF_FAIL_ARGS_NO_VERBOSE(...)                             \
  do {                                                                         \
    if (!(VALID_ARGS(__VA_ARGS__))) {                                          \
      PRINT_ERR_1ST_NULL_ARG(__VA_ARGS__);                                     \
      return ASM_INVALID_ARGS;                                                 \
    }                                                                          \
  } while (0)

#define RETURN_PRINT_ERR_IF_FAIL(err, ...)                                     \
  do {                                                                         \
    if (!(VALID_ARGS(__VA_ARGS__))) {                                          \
      PRINT_ERR_1ST_NULL_ARG(__VA_ARGS__);                                     \
      return err;                                                              \
    }                                                                          \
  } while (0)

#endif
