#ifndef LEXER_H
#define LEXER_H
/* Module for tokenizing any line. From char[] create Token[]. */

#include <stddef.h>

#include "common.h"

// All possible types of token.
enum Token_Type {
  TOKEN_INSTRUCTION,  // instruction name in .CODE segment
  TOKEN_REGISTER,     // register name
  TOKEN_NUMBER,       // any numeric value
  TOKEN_IDENTIFIER,   // variable name
  TOKEN_LABEL,        // label name (with @ prefix)
  TOKEN_COMMA,        // literally comma,
  TOKEN_SECTION_DATA, // .DATA
  TOKEN_SECTION_CODE, // .CODE
  TOKEN_KMA,          // .KMA
  TOKEN_OFFSET,       // keyword used in code segment
  TOKEN_QUESTION,     // question mark, used in data segment ?
  TOKEN_STRING,       // any string inside double quotes
  TOKEN_DATA_TYPE,    // DB, DWORD, ... in data segment
  TOKEN_DUP,          // keyword in data segment
  TOKEN_LPAREN,       // left parenthesis
  TOKEN_RPAREN,       // right parenthesis
  TOKEN_EOF,          // end of file/line
  TOKEN_UNKNOWN       // unknown token = probably syntax error
};

// Representation of one token with type and copy of original string value
struct Token {
  enum Token_Type type;
  char value[TOKEN_MAX_VALUE_LEN];
  size_t line_number;
};

// Tokenize given <line> (ended by \0).
// Return pointer to array of tokens, ended by TOKEN_EOF, this array must be
// later freed by calling lexer_free_tokens. Return NULL on failure.
struct Token *lexer_tokenize_line(const char *line, const size_t nl);

// Free token array created by tokenizing one line.
// Can be used on any token array as long as it is ended by TOKEN_EOF
void lexer_free_tokens(struct Token *tokens);

// Return string representation of a given token type. Useful for prints
const char *token_type_to_str(enum Token_Type type);

// Print one given token to stdout.
void print_token(const struct Token *token);

// Print array of tokens, until TOKEN_EOF is found.
// If EOF is not found, may result in segfault.
void print_tokens(const struct Token *tokens);

#endif
