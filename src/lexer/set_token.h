#ifndef SET_TOKEN_H
#define SET_TOKEN_H

#include <stddef.h>

#include "lexer.h"

// ===== MACROS =====

#define IDENTIFY(ch, type)                                                     \
  if (num == sizeof(ch) - 1 && strncmp(word, (ch), num) == 0) {                \
    return (type);                                                             \
  }

// Based on current position in the line, update the given token.
// Update pos to one char after token characters.
// Return 1 on success, 0 on failure.
int set_next_token(struct Token *token, const char *line, const size_t len,
                   size_t *pos, const size_t nl);

// Update token to have given parameters.
// Value can be NULL, in that case a token->value is irrelevant.
// Return 1 on success, 0 on failure.
int set_token(struct Token *token, const enum Token_Type type,
              const char *value, const size_t nl);

// Update token to have given parameters.
// DOESN'T SUPPORT 0-length value!
// Only uses first len characters from given value = good for non-NULL
// terminated strings. It terminates the token->value by itself! Set Return 1 on
// success, 0 on failure.
int set_token_len(struct Token *token, const enum Token_Type type,
                  const char *value, const size_t nl, const size_t len);

// The string in DATA segments. Takes pointer AFTER first QUOTE.
// Update the given token to have all info.
// Return 1 on success, 0 on failure.
int set_token_string(struct Token *token, const char *s, const size_t nl);

// Take pointer to the '@'.
// Set token to updated values.
// Return 1 on success, 0 on failure.
int set_token_label(struct Token *token, const char *s, const size_t nl);

// Take pointer to first digit (or minus sign).
// Set token for  number, found in code.
// Return 1 on success, 0 on failure.
int set_token_number(struct Token *token, const char *s, const size_t nl);

// Take pointer to first letter.
// Distinguish between different words and set token to the one which it is.
// Return 1 on success, 0 on failure.
int set_token_word(struct Token *token, const char *s, const size_t nl);

// Classify the first <num> number of characters of word.
// Return the adequate TokenType, or TOKEN_UNKNOWN if any error.
enum Token_Type lexer_classify_word(const char *word, const size_t num);

#endif
