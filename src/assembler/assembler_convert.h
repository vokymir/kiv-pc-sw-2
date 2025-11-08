#ifndef ASSEMBLER_CONVERT_H
#define ASSEMBLER_CONVERT_H
/* Module helper for converting - 3 main areas:
 * 1. numeric conversion
 * 2. error conversion
 * 3. modules mismatch, lexer & parser work with array of Tokens, but
 * differently*/

#include <stddef.h>
#include <stdint.h>

#include "assembler.h"
#include "lexer.h"

// === CONVERTING ===

// = 1. numeric =

// safe wrapper of convertor from uint32_t to int32_t
int32_t convert_uint32(uint32_t u);

// safe wrapper of conversion from size_t to int32_t
int32_t convert_size_t(size_t s);

// = 2. error =

// Given any ASM error, convert it to corresponding MAIN error.
enum Err_Main convert_err(enum Err_Asm err);

// = 3. modules mismatch =

// I accidentally created a continuous array of tokens in lexer but require
// array of pointers to tokens in parser. This function is a bridge between
// these differences. Caller must free this "convertor" after is used. Return
// NULL on failure, pointer on success.
const struct Token **convert_tokens_to_arr(const struct Token *orig);

// Free the array of pointers to tokens & set the pointer to null.
// It frees only the array, not the tokens = can have const ptr.
void convert_free_tokens_arr(const struct Token **tokens[]);

// Convert the structure given from lexer (struct Token *) to that wanted in
// parser (struct Token **). Free that structure on return. This function is
// only a wrapper around parse_tokens in parser.
struct Parsed_Statement *convert_parse_tokens(const struct Token *tokens,
                                              size_t nl);

#endif
