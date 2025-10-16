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

// Free tokens insides = clear value, DOESNT free the token itself.
void lexer_free_token_inside(struct Token *token);

// Free token inside, then free the token itself;
void lexer_free_token(struct Token *token);

// Skip all whitespaces or comments in line by INCREMENTING the pos value.
// Return 1 if there is a token waiting to be parsed on pos.
// Return 0 if end of line was reached.
int _lexer_skip_to_next_token(const char *line, const size_t len, size_t *pos);

// Create next token, starting on pos.
// Update pos to one char after token characters.
// Return Token on success, NULL on failure.
struct Token *_lexer_create_next_token(const char *line, size_t len,
                                       size_t *pos, size_t nl);

// Add a token to the list.
// Return 1 on success, 0 on failure.
// On failure, the token IS FREED.
int _lexer_add_token_to_list(struct DS_Llist *tokens_list, struct Token *token);

// Create token with given parameters, memcpy value using strlen().
// Return pointer to token on success, NULL on failure.
// Caller must free.
struct Token *_lexer_create_token(const enum TokenType type, const char *value,
                                  const size_t nl);

// Create token with given parameters, memcpy the value using len. If you want
// to end it eg by \0, you must pass (real-len+1) and manually rewrite it.
// Return pointer to token on success, NULL on failure. Caller must free.
struct Token *_lexer_create_token_n(const enum TokenType type,
                                    const char *value, const size_t nl,
                                    const size_t len);

// The string in DATA segments. Takes pointer AFTER first QUOTE.
// Return pointer to token on success, NULL on failure.
// Caller must free.
struct Token *_lexer_create_token_string(const char *s, const size_t nl);

// Create token from pointer to the start of label (starting with @).
// Take pointer to the '@'.
// Return pointer to Token or NULL.
// Caller must free.
struct Token *_lexer_create_token_label(const char *s, const size_t nl);

// Create token for static number, found wherever in code.
// Take pointer to first digit (or minus sign).
// Return pointer to Token or NULL.
// Caller must free.
struct Token *_lexer_create_token_number(const char *s, const size_t nl);

// Distinguish between different words and return which one it is.
// Take pointer to first letter.
// Return pointer to Token or NULL.
// Caller must free.
struct Token *_lexer_create_token_word(const char *s, const size_t nl);

#endif
