#include <errno.h>
#include <inttypes.h>
#include <string.h>

#include "common.h"
#include "parser_token.h"

int token_is(const struct Token *tok, enum Token_Type type) {
  return (tok && tok->type == type);
}

int token_is_eof(const struct Token *tok) {
  return (tok && tok->type == TOKEN_EOF);
}

int tokens_start_with(const struct Token *tokens[], size_t n,
                      const enum Token_Type types[]) {
  size_t i = 0;
  if (!tokens || !*tokens || !types) {
    PRINT_ERR("Invalid argumnets.");
    return 0;
  }

  for (i = 0; i < n; i++) {
    if (!token_is(tokens[i], types[i])) {
      return 0;
    }
    if (i < n - 1 && token_is_eof(tokens[i])) {
      return 0; // don't go past the end of array
    }
  }

  return 1;
}

// ===== STRING HELPER DEFINITIONS =====

int token_copy_value(const struct Token *token, char *dest, size_t len) {
  size_t copy_len = len - 1;
  RET_STDERR_IF_FAIL(token && dest && copy_len > 0, 0, "Invalid arguments.");

  memcpy(dest, token->value, copy_len);
  dest[copy_len] = '\0';

  return 1;
}

int token_value_eq(const struct Token *token, const char *s) {
  RET_STDERR_IF_FAIL(token && s, 0, "Invalid arguments.");

  return (strcmp(token->value, s) == 0);
}

// ===== NUMBER HELPER DEFINITIONS =====

int token_parse_int32(const struct Token *token, int32_t *out) {
  intmax_t res = 0;
  char *end = NULL;
  RET_STDERR_IF_FAIL(token && out, 0,
                     "Tried parse int32 but invalid arguments.");

  errno = 0;
  res = strtoimax(token->value, &end, 10);
  if (end == token->value)
    return 0; // no digits parsed
  if (errno == ERANGE)
    return 0; // out of range
  if (res < INT32_MIN || res > INT32_MAX)
    return 0;

  *out = (int32_t)res;
  return 1;
}

int token_parse_size_t(const struct Token *token, size_t *out) {
  unsigned long long res = 0;
  char *end = NULL;
  RET_STDERR_IF_FAIL(token && out, 0,
                     "Tried parse size_t but invalid arguments.");

  errno = 0;
  res =
      strtoull(token->value, &end, 10); // unsigned long long is bigger > size_t
  if (end == token->value)
    return 0; // no digits
  if (errno == ERANGE)
    return 0; // overflow
  if (res > SIZE_MAX)
    return 0; // bigger > size_t

  *out = (size_t)res;
  return 1;
}
