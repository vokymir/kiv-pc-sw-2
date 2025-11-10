#ifndef PARSER_TOKEN_HELPERS
#define PARSER_TOKEN_HELPERS
/* Module helper for working with tokens in parser. */

#include <stdint.h>

#include "lexer.h"

// Check if token is not null and its type is <type>.
// Return 1 on success, 0 on failure.
int token_is(const struct Token *token, enum Token_Type type);

// Check if token is not null and its type is EOF.
// Return 1 on success, 0 on failure.
int token_is_eof(const struct Token *token);

// Check if first n tokens exist and are exact types.
// Return 1 on success, 0 on failure.
int tokens_start_with(const struct Token *tokens[], size_t n,
                      const enum Token_Type types[]);

// ===== STRING HELPER DECLARATIONS =====

// copy <token>->value into <dest> and NULL-terminates the string
// Return 1 on success, 0 on failure.
int token_copy_value(const struct Token *token, char *dest, size_t len);

// Safe wrapper of strcmp(token->value, s)
// Return 1 on success, 0 on failure.
int token_value_eq(const struct Token *token, const char *s);

// ===== NUMBER HELPER DECLARATIONS =====

// Parse <token>->value and save it into <out>.
// Return 1 on success, 0 on failure.
int token_parse_int32(const struct Token *token, int32_t *out);

// Parse <token>->value and save it into <out>.
// Return 1 on success, 0 on failure.
int token_parse_size_t(const struct Token *token, size_t *out);

#endif
