#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

#define TOKEN_MAX_VALUE_LEN 256

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
  char value[TOKEN_MAX_VALUE_LEN];
  size_t line_number;
};

// Tokenize given line (ended by \0).
// Return pointer to array of tokens, ended by TOKEN_EOF, this array must be
// later freed by calling lexer_free_tokens. Return NULL on failure.
struct Token *lexer_tokenize_line(const char *line, const size_t nl);

// Free token array created by tokenizing one line.
void lexer_free_tokens(struct Token *tokens);

// Print one given token
void print_token(const struct Token *token);

// Print array of tokens, until TOKEN_EOF is found.
// If EOF is not found, may result in segfault.
void print_tokens(const struct Token *tokens);

#endif
