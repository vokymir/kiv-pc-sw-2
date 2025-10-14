#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

// All possible types of token.
enum TokenType {
  TOKEN_INSTRUCTION,
  TOKEN_REGISTER,
  TOKEN_NUMBER,
  TOKEN_IDENTIFIER,
  TOKEN_LABEL,
  TOKEN_COMMA,
  TOKEN_SECTION_DATA,
  TOKEN_SECTION_CODE,
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
  enum TokenType type;
  char *value;
  size_t line_number;
};

// Tokenize given line (ended by \0).
// Return pointer to array of tokens, ended by TOKEN_EOF, or return NULL on
// failure.
struct Token *lexer_tokenize_line(const char *line, const size_t nl);

// Free an array of tokens. Must be ended by TOKEN_EOF.
void lexer_free_tokens(struct Token *tokens);

#endif
