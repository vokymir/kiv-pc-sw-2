#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#include "datastruc.h"

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
// Return pointer to list of tokens, ended by TOKEN_EOF.
// Return NULL on failure.
struct DS_Llist *lexer_tokenize_line(const char *line, const size_t nl);

// Free a list of tokens.
void lexer_free_tokens(struct Token *tokens);

// Create token with given parameters
// Return pointer to token on success, NULL on failure.
// Caller must free.
struct Token *_lexer_create_token(const enum TokenType type, const char *value,
                                  const int nl);

// Get what type of string it is & save in Token. Return NULL on failure.
// The string in DATA segments.
struct Token *_lexer_parse_string(const char *);

// Get number & save to Token.
struct Token *_lexer_parse_number(const char *);

// Get label & save to Token
struct Token *_lexer_parse_label(const char *);

// Get identifier & save to Token
struct Token *_lexer_parse_word(const char *);

#endif
