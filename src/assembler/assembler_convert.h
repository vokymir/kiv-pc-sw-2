#ifndef ASSEMBLER_CONVERT_H
#define ASSEMBLER_CONVERT_H

#include "assembler.h"
#include "lexer.h"
#include <stddef.h>
#include <stdint.h>

// === CONVERTING ===

// safe wrapper of convertor from uint32_t to int32_t
int32_t convert_uint32(uint32_t u);

// safe wrapper of conversion from size_t to int32_t
int32_t convert_size_t(size_t s);

// Given any ASM error, convert it to corresponding MAIN error.
enum Err_Main convert_err(enum Err_Asm err);

// I accidentally created a continuous array of tokens in lexer but require
// array of pointers to tokens in parser. This function is a bridge between
// these differences. Caller must free this "convertor" after is used. Return
// NULL on failure, pointer on success.
const struct Token **convert_tokens_to_arr(const struct Token *orig);

// Free the array of pointers to tokens & set the pointer to null.
// It frees only the array, not the tokens = can have const ptr.
void convert_free_tokens_arr(const struct Token **tokens[]);

// Convert the structure given from lexer (struct Token *) to that wanted in
// parser (struct Token **). Free that structure on return. Otherwise only a
// wrapper around parse_tokens in parser.
struct Parsed_Statement *convert_parse_tokens(const struct Token *tokens,
                                              size_t nl);

#endif
