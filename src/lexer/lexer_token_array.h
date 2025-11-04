#ifndef TOKEN_ARRAY_H
#define TOKEN_ARRAY_H

#include <stddef.h>

struct Token_Arr {
  struct Token *tokens;
  size_t count;
  size_t capacity;
};

// Initialize the insides of token array.
// Return 1 on success, 0 on failure.
int tok_arr_init(struct Token_Arr *arr);

// Free all insides of token array.
void tok_arr_deinit(struct Token_Arr *arr);

// Ensure that in the token array is enough space for additional tokens.
// Return 1 on success, 0 on failure.
int tok_arr_ensure_capacity(struct Token_Arr *arr, size_t additional_tokens);

#endif
