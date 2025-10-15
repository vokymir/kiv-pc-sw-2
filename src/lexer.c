#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "datastruc.h"
#include "lexer.h"
#include "memory.h"

struct DS_Llist *lexer_tokenize_line(const char *line, const size_t nl) {
  struct DS_Llist *tokens = NULL;
  struct Token *token = NULL;
  enum TokenType type = TOKEN_EOF;
  char *value = NULL;
  void *tmp = NULL;
  int pos, len = 0;
  if (!line) {
    return NULL;
  }

  tokens = ds_llist_new(sizeof(struct Token));
  if (!tokens) {
    return NULL;
  }
  len = strlen(line);

  while (pos < len) {
    while (pos < len && isspace(line[pos])) { // skip whitespace
      pos++;
    }

    if (pos >= len) { // End Of Line
      break;
    }

    if (line[pos] == ';') { // Comment
      break;
    }

    // TODO: break this switch into IFs, more flexible on alphanum/digit words
    // etc.
    switch (line[pos]) {
    case ',':
      type = TOKEN_COMMA;
      value = ",";
      break;
    case '(':
      type = TOKEN_LPAREN;
      value = "(";
      break;
    case ')':
      type = TOKEN_RPAREN;
      value = ")";
      break;
    case '?':
      type = TOKEN_QUESTION;
      value = "?";
      break;
    case '"':
      // TODO: extract string and jump more pos
      break;
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      // TODO: extract number and jump more pos
      break;
    case '@':
      // TODO: extract label and jump more pos
      break;
    default:
      if (!isalpha(line[pos] || line[pos] == '.')) {
        type = TOKEN_UNKNOWN;
        // TODO: there is an error
        break;
      }
      // TODO: extract word and classify
    }

    pos++;
    token = _lexer_create_token(type, value, nl);
    if (!token) { // TODO: really? don't leak some memory?
      return NULL;
    }
    tmp = ds_llist_add(tokens, token);
    if (!tmp) {
      return NULL; // TODO: really? leaking memory...
    }
    jree(token);
  }

  // TODO: last token
  {
    token = _lexer_create_token(TOKEN_EOF, "", nl);
    if (!token) { // TODO: really? don't leak some memory?
      return NULL;
    }
    tmp = ds_llist_add(tokens, token);
    if (!tmp) {
      return NULL; // TODO: really? leaking memory...
    }
    jree(token);
  }

  return tokens;
}

void lexer_free_tokens(struct Token *tokens);

struct Token *_lexer_create_token(const enum TokenType type, const char *value,
                                  const int nl);
