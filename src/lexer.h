#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#include "llist.h"

// All possible types of token.
enum Token_Type {
  TOKEN_INSTRUCTION,
  TOKEN_REGISTER,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_LABEL,
  TOKEN_COMMA,
  TOKEN_SECTION_DATA,
  TOKEN_SECTION_CODE,
  TOKEN_KMA,
  TOKEN_OFFSET,
  TOKEN_QUESTION,
  TOKEN_STRING,
  TOKEN_DATA_TYPE,
  TOKEN_DUP,
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_EOF,
  TOKEN_UNKNOWN
};

// Representation of one token.
struct Token {
  enum Token_Type type;
  char *value;
  size_t line_number;
};

// Tokenize given line (ended by \0).
// Return pointer to list of tokens, ended by TOKEN_EOF.
// Return NULL on failure.
struct Llist *lexer_tokenize_line(const char *line, const size_t nl);

// Free tokens insides, then free the token itself;
void lexer_free_token(struct Token *token);

#endif
