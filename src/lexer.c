#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "lexer.h"
#include "llist.h"
#include "memory.h"

#define IDENTIFY(ch, type)                                                     \
  if (num == sizeof(ch) - 1 && strncmp(word, (ch), num) == 0) {                \
    return (type);                                                             \
  }

struct Llist *lexer_tokenize_line(const char *line, const size_t nl) {
  struct Llist *tokens = NULL;
  struct Token *token = NULL;
  llist_free_node_data func = (llist_free_node_data)lexer_free_token;
  size_t pos = 0;
  size_t len = 0;

  if (!line) {
    return NULL;
  }

  tokens = llist_create(sizeof(struct Token));
  if (!tokens) {
    return NULL;
  }

  len = strlen(line);

  // Process the whole line
  while (_lexer_skip_to_next_token(line, len, &pos)) {
    token = _lexer_create_next_token(line, len, &pos, nl);

    if (!_lexer_add_token_to_list(tokens, token)) {
      llist_free(tokens, func);
      return NULL;
    }
    token = NULL;
  }

  // Add EOF to the end
  token = _lexer_create_token(TOKEN_EOF, "", nl);
  if (!_lexer_add_token_to_list(tokens, token)) {
    llist_free(tokens, func);
    return NULL;
  }

  return tokens;
}

void lexer_free_token(struct Token *token) {
  if (!token) {
    return;
  }
  if (token->value) {
    jree(token->value);
  }
  jree(token);
  return;
}

int _lexer_skip_to_next_token(const char *line, const size_t len, size_t *pos) {
  if (!line || !pos) {
    return 0;
  }

  if (*pos >= len) {
    *pos = len;
    return 0;
  }

  while (*pos < len && isspace((unsigned char)line[*pos])) { // skip whitespaces
    (*pos)++;
  }

  if (*pos >= len) { // reached EO Line
    *pos = len;
    return 0;
  }

  if (line[*pos] == ';') { // reached comments
    return 0;
  }
  return 1; // found something meaningful
}

struct Token *_lexer_create_next_token(const char *line, const size_t len,
                                       size_t *pos, const size_t nl) {
  struct Token *token = NULL;
  char current = 0;
  size_t jmp = 0;
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
    token = _lexer_create_token_string(&line[*pos + 1], nl);
    if (token && token->value) {
      jmp = strlen(token->value); // string literal can be 0-length
      (*pos) += jmp + 2;          // +2 for the quotes on begin/end
    } else {
      if (token) {
        jree(token);
      }
      return NULL;
    }
    return token;
  }

  // Label (starts with @)
  if (current == '@') {
    token = _lexer_create_token_label(&line[*pos], nl);
    if (token && token->value && (jmp = strlen(token->value)) > 0) {
      (*pos) += jmp;
    } else {
      if (token) {
        jree(token);
      }
      return NULL;
    }
    return token;
  }

  // Number (digit or negative number)
  if (isdigit(current) ||
      (current == '-' && *pos + 1 < len && isdigit(line[*pos + 1]))) {
    token = _lexer_create_token_number(&line[*pos], nl);
    if (token && token->value && (jmp = strlen(token->value)) > 0) {
      (*pos) += jmp;
    } else {
      if (token) {
        jree(token);
      }
      return NULL;
    }
    return token;
  }

  // Word (instruction, register, keyword, or identifier)
  if (isalpha(current) || current == '.') {
    token = _lexer_create_token_word(&line[*pos], nl);
    if (token && token->value && (jmp = strlen(token->value)) > 0) {
      (*pos) += jmp;
    } else {
      if (token) {
        jree(token);
      }
      return NULL;
    }
    return token;
  }

  // Unknown character
  token = _lexer_create_token(TOKEN_UNKNOWN, &line[*pos], nl);
  (*pos)++;
  return token;
}

int _lexer_add_token_to_list(struct Llist *tokens_list, struct Token *token) {
  if (!tokens_list || !token) {
    return 0;
  }

  if (!token->value) {
    lexer_free_token(token);
    return 0;
  }

  if (!llist_add(tokens_list, (void **)&token)) {
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

  while (*curr && *curr != '"') {
    n_chars++;
    curr++;
  }
  if (*curr == '\0') {
    return NULL; // unterminated string
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
  if (!s || *s != '@') {
    return NULL;
  }
  n_chars++; // the @ at the beginning
  curr++;

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  if (n_chars < 2) { // the symbol @ is invalid
    return NULL;
  }

  token = _lexer_create_token_n(TOKEN_LABEL, s, nl, n_chars + 1); // 1 for \0
  if (!token) {
    return NULL;
  }
  token->value[n_chars] = '\0';

  return token;
}

struct Token *_lexer_create_token_number(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  if (!s) {
    return NULL;
  }

  if (*curr == '-') { // optional negative number
    n_chars++;
    curr++;
  }

  while (isdigit(*curr)) {
    n_chars++;
    curr++;
  }

  token = _lexer_create_token_n(TOKEN_NUMBER, s, nl, n_chars + 1); // 1 for \0
  if (!token) {
    return NULL;
  }
  token->value[n_chars] = '\0';

  return token;
}

struct Token *_lexer_create_token_word(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  enum TokenType type = TOKEN_UNKNOWN;
  if (!s) {
    return NULL;
  }

  if (*curr && *curr == '.') {
    n_chars++;
    curr++;
  }

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  type = _lexer_classify_word(s, n_chars);
  token = _lexer_create_token_n(type, s, nl, n_chars + 1); // 1 for \0
  if (!token) {
    return NULL;
  }
  token->value[n_chars] = '\0';

  return token;
}

enum TokenType _lexer_classify_word(const char *word, const size_t num) {
  if (!word || num == 0) {
    return TOKEN_UNKNOWN;
  }
  // SECTION MARKER
  IDENTIFY(".KMA", TOKEN_KMA);
  IDENTIFY(".DATA", TOKEN_SECTION_DATA);
  IDENTIFY(".CODE", TOKEN_SECTION_CODE);
  // DATA TYPE
  IDENTIFY("DW", TOKEN_DATA_TYPE);
  IDENTIFY("DWORD", TOKEN_DATA_TYPE);
  IDENTIFY("DB", TOKEN_DATA_TYPE);
  IDENTIFY("BYTE", TOKEN_DATA_TYPE);
  // REGISTERS
  IDENTIFY("A", TOKEN_REGISTER);
  IDENTIFY("B", TOKEN_REGISTER);
  IDENTIFY("C", TOKEN_REGISTER);
  IDENTIFY("D", TOKEN_REGISTER);
  IDENTIFY("S", TOKEN_REGISTER);
  IDENTIFY("SP", TOKEN_REGISTER);
  // INSTRUCTIONS
  IDENTIFY("HALT", TOKEN_INSTRUCTION);
  IDENTIFY("NOP", TOKEN_INSTRUCTION);

  IDENTIFY("MOV", TOKEN_INSTRUCTION);
  IDENTIFY("MOVSD", TOKEN_INSTRUCTION);
  IDENTIFY("LOAD", TOKEN_INSTRUCTION);
  IDENTIFY("STOR", TOKEN_INSTRUCTION);

  IDENTIFY("PUSH", TOKEN_INSTRUCTION);
  IDENTIFY("POP", TOKEN_INSTRUCTION);

  IDENTIFY("ADD", TOKEN_INSTRUCTION);
  IDENTIFY("SUB", TOKEN_INSTRUCTION);
  IDENTIFY("MUL", TOKEN_INSTRUCTION);
  IDENTIFY("DIV", TOKEN_INSTRUCTION);
  IDENTIFY("INC", TOKEN_INSTRUCTION);
  IDENTIFY("DEC", TOKEN_INSTRUCTION);

  IDENTIFY("AND", TOKEN_INSTRUCTION);
  IDENTIFY("OR", TOKEN_INSTRUCTION);
  IDENTIFY("XOR", TOKEN_INSTRUCTION);
  IDENTIFY("NOT", TOKEN_INSTRUCTION);

  IDENTIFY("SHL", TOKEN_INSTRUCTION);
  IDENTIFY("SHR", TOKEN_INSTRUCTION);

  IDENTIFY("CMP", TOKEN_INSTRUCTION);

  IDENTIFY("JMP", TOKEN_INSTRUCTION);
  IDENTIFY("JE", TOKEN_INSTRUCTION);
  IDENTIFY("JNE", TOKEN_INSTRUCTION);
  IDENTIFY("JG", TOKEN_INSTRUCTION);
  IDENTIFY("JGE", TOKEN_INSTRUCTION);
  IDENTIFY("JNG", TOKEN_INSTRUCTION);
  IDENTIFY("JL", TOKEN_INSTRUCTION);
  IDENTIFY("JLE", TOKEN_INSTRUCTION);
  IDENTIFY("JNL", TOKEN_INSTRUCTION);

  IDENTIFY("CALL", TOKEN_INSTRUCTION);
  IDENTIFY("RET", TOKEN_INSTRUCTION);

  IDENTIFY("OUTD", TOKEN_INSTRUCTION);
  IDENTIFY("OUTC", TOKEN_INSTRUCTION);
  IDENTIFY("OUTS", TOKEN_INSTRUCTION);
  IDENTIFY("INPD", TOKEN_INSTRUCTION);
  IDENTIFY("INPC", TOKEN_INSTRUCTION);
  IDENTIFY("INPS", TOKEN_INSTRUCTION);
  // SPECIALS
  IDENTIFY("DUP", TOKEN_DUP);
  IDENTIFY("OFFSET", TOKEN_OFFSET);

  return TOKEN_IDENTIFIER;
}
