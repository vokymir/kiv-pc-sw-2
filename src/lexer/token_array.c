#include "common.h"
#include "lexer.h"
#include "memory.h"

#include "lexer_token_array.h"

int tok_arr_init(struct Token_Arr *arr) {
  CLEANUP_IF_FAIL_ERR(arr, "Tried to initialize token array from NULL.");

  arr->tokens = jalloc(TOKENS_INITIAL_CAPACITY * sizeof(struct Token));
  CLEANUP_IF_FAIL_ERR(arr->tokens, "Couldn't allocate token array.");

  arr->count = 0;
  arr->capacity = TOKENS_INITIAL_CAPACITY;

  return 1;

cleanup:
  return 0;
}

void tok_arr_deinit(struct Token_Arr *arr) {
  CLEANUP_IF_FAIL_ERR(arr, "Tried to de-initialize token array from NULL.");

  if (arr->tokens) {
    jree(arr->tokens);
    arr->tokens = NULL;
  }
  arr->count = 0;
  arr->capacity = 0;

cleanup:
  return;
}

int tok_arr_ensure_capacity(struct Token_Arr *arr, size_t additional_tokens) {
  size_t req = 0, new_cap = 0;
  struct Token *new_tokens = NULL;
  CLEANUP_IF_FAIL_ERR(arr, "Tried to ensure capacity of token array NULL.");

  if (additional_tokens == 0) {
    return 1;
  }

  req = arr->count + additional_tokens;
  if (req <= arr->capacity) {
    return 1;
  }

  new_cap = arr->capacity ? arr->capacity : TOKENS_INITIAL_CAPACITY;
  while (new_cap < req) {
    new_cap *= TOKENS_CAPACITY_MULT;
  }

  new_tokens = jealloc(arr->tokens, new_cap * sizeof(struct Token));
  CLEANUP_IF_FAIL_ERR(new_tokens, "Couldn't grow token array using realloc.");

  arr->tokens = new_tokens;
  arr->capacity = new_cap;
  return 1;

cleanup:
  return 0;
}
