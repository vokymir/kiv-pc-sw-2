#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "datastruc.h"
#include "lexer.h"
#include "memory.h"

struct DS_Llist *lexer_tokenize_line(const char *line, const size_t nl) {
  struct DS_Llist *tokens = NULL;
  struct Token *token = NULL;
  ds_llist_free_it_func func = (ds_llist_free_it_func)lexer_free_token_inside;
  size_t pos = 0;
  size_t len = 0;

  if (!line) {
    return NULL;
  }

  tokens = ds_llist_new(sizeof(struct Token));
  if (!tokens) {
    return NULL;
  }

  len = strlen(line);

  // Process the whole line
  while (!_lexer_skip_to_next_token(line, len, &pos)) {
    token = _lexer_create_next_token(line, len, &pos, nl);

    if (!_lexer_add_token_to_list(tokens, token)) {
      ds_llist_free(tokens, func);
      return NULL;
    }
    token = NULL;
  }

  // Add EOF to the end
  token = _lexer_create_token(TOKEN_EOF, "", nl);
  if (!_lexer_add_token_to_list(tokens, token)) {
    ds_llist_free(tokens, func);
    return NULL;
  }

  return tokens;
}

void lexer_free_token_inside(struct Token *token) {
  if (!token || !token->value) {
    return;
  }
  jree(token->value);
  return;
}

void lexer_free_token(struct Token *token) {
  if (!token) {
    return;
  }
  lexer_free_token_inside(token);
  jree(token);
  return;
}

int _lexer_skip_to_next_token(const char *line, const size_t len, size_t *pos) {
  if (!line || !pos || len == 0 || len <= *pos) {
    return 0;
  }

  while (*pos < len && isspace(line[*pos])) { // skip whitespaces
    (*pos)++;
  }

  if (*pos > len) { // reached EO Line
    return 0;
  }

  if (line[*pos] == ';') { // reached comments
    return 0;
  }

  return 1; // found something meaningful
}

struct Token *_lexer_create_next_token(const char *line, size_t len,
                                       size_t *pos, size_t nl) {
  struct Token *token = NULL;
  char current = 0;
  if (!line || !pos || len == 0 || len <= *pos) {
    return NULL;
  }

  current = line[*pos];

  // Single-character tokens
  if (current == ',') {
    token = _lexer_create_token(TOKEN_COMMA, ",", nl);
    (*pos)++;
    return token;
  }

  if (current == '(') {
    token = _lexer_create_token(TOKEN_LPAREN, "(", nl);
    (*pos)++;
    return token;
  }

  if (current == ')') {
    token = _lexer_create_token(TOKEN_RPAREN, ")", nl);
    (*pos)++;
    return token;
  }

  if (current == '?') {
    token = _lexer_create_token(TOKEN_QUESTION, "?", nl);
    (*pos)++;
    return token;
  }

  // String literal in .DATA segment
  if (current == '"') {
    token = _lexer_create_token_string(&line[*pos], nl);
    if (token && token->value) {
      (*pos) += strlen(token->value) + 2; // +2 for the quotes on begin/end
    }
    return token;
  }

  // Label (starts with @)
  if (current == '@') {
    token = _lexer_create_token_label(&line[*pos], nl);
    if (token && token->value) {
      (*pos) += strlen(token->value);
    }
    return token;
  }

  // Number (digit or negative number)
  if (isdigit(current) ||
      (current == '-' && *pos + 1 < len && isdigit(line[*pos + 1]))) {
    token = _lexer_create_token_number(&line[*pos], nl);
    if (token && token->value) {
      (*pos) += strlen(token->value);
    }
    return token;
  }

  // Word (instruction, register, keyword, or identifier)
  if (isalpha(current) || current == '.') {
    token = _lexer_create_token_word(&line[*pos], nl);
    if (token && token->value) {
      (*pos) += strlen(token->value);
    }
    return token;
  }

  // Unknown character
  token = _lexer_create_token(TOKEN_UNKNOWN, &line[*pos], nl);
  (*pos)++;
  return token;
}

int _lexer_add_token_to_list(struct DS_Llist *tokens_list,
                             struct Token *token) {
  if (!tokens_list || !token) {
    return 0;
  }

  if (!token->value) {
    lexer_free_token(token);
    return 0;
  }

  if (!ds_llist_add(tokens_list, token)) {
    lexer_free_token(token);
    return 0;
  }

  return 1;
}

struct Token *_lexer_create_token(const enum TokenType type, const char *value,
                                  const size_t nl) {
  size_t val_len = strlen(value) + 1;
  struct Token *token = jalloc(sizeof(struct Token));
  if (!token) {
    return NULL;
  }
  token->value = jalloc(sizeof(char) * val_len);
  if (!token->value) {
    jree(token);
    return NULL;
  }

  token->type = type;
  token->line_number = nl;
  memcpy(token->value, value, val_len);

  return token;
}

struct Token *_lexer_create_token_n(const enum TokenType type,
                                    const char *value, const size_t nl,
                                    const size_t len) {
  struct Token *token = jalloc(sizeof(struct Token));
  if (!token) {
    return NULL;
  }
  token->value = jalloc(sizeof(char) * len);
  if (!token->value) {
    jree(token);
    return NULL;
  }

  token->type = type;
  token->line_number = nl;
  memcpy(token->value, value, len);

  return token;
}

struct Token *_lexer_create_token_string(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  if (!s) {
    return NULL;
  }

  while (*curr != '"') {
    n_chars++;
    curr = curr + 1;
  }

  token = _lexer_create_token_n(TOKEN_STRING, s, nl, n_chars + 1); // 1 for \0
  if (!token) {
    return NULL;
  }
  token->value[n_chars] = '\0';

  return token;
}

struct Token *_lexer_create_token_label(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  if (!s) {
    return NULL;
  }

  while (!(isspace(*curr) || *curr == ':')) {
    n_chars++;
    curr = curr + 1;
  }

  token = _lexer_create_token_n(TOKEN_LABEL, s, nl, n_chars + 1); // 1 for \0
  if (!token) {
    return NULL;
  }
  token->value[n_chars] = '\0';

  return NULL;
}

struct Token *_lexer_create_token_number(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  if (!s) {
    return NULL;
  }

  while (isdigit(curr)) {
    n_chars++;
    curr = curr + 1;
  }

  token = _lexer_create_token_n(TOKEN_NUMBER, s, nl, n_chars + 1); // 1 for \0
  if (!token) {
    return NULL;
  }
  token->value[n_chars] = '\0';

  return NULL;
}

struct Token *_lexer_create_token_word(const char *s, const size_t nl) {
  return NULL;
}
