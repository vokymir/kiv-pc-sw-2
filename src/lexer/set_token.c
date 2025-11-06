#include <ctype.h>
#include <string.h>

#include "instruction.h"

#include "lexer_set_token.h"

// shortcut for identifying word with token types
#define IDENTIFY(ch, type)                                                     \
  if (num == sizeof(ch) - 1 && strncmp(word, (ch), num) == 0) {                \
    return (type);                                                             \
  }

int set_next_token(struct Token *token, const char *line, const size_t len,
                   size_t *pos, const size_t nl) {
  const char *current = NULL;
  CLEANUP_IF_FAIL(token && line && pos && len > 0 && len > *pos,
                  "Tried setting next token, but gave NULL argument.");

  current = &line[*pos];

  // Single-character tokens
  if (*current == ',') {
    (*pos)++;
    return set_token(token, TOKEN_COMMA, ",", nl);
  }

  if (*current == '(') {
    (*pos)++;
    return set_token(token, TOKEN_LPAREN, "(", nl);
  }

  if (*current == ')') {
    (*pos)++;
    return set_token(token, TOKEN_RPAREN, ")", nl);
  }

  if (*current == '?') {
    (*pos)++;
    return set_token(token, TOKEN_QUESTION, "?", nl);
  }

  // String literal in .DATA segment
  if (*current == '"') {
    CLEANUP_IF_FAIL(
        set_token_string(token, current + 1, nl),
        "Couldn't set token to string literal."); // +1 for opening quote
    (*pos) += strlen(token->value) + 2; // +2 for the quotes on begin/end
    return 1;
  }

  // Label (starts with @)
  if (*current == '@') {
    CLEANUP_IF_FAIL(set_token_label(token, current, nl),
                    "Couldn't set token to label.");
    (*pos) += strlen(token->value) + 1; // +1 for ':'
    return 1;
  }

  // Number (digit or negative number)
  if (isdigit(*current) ||
      (*current == '-' && *pos + 1 < len && isdigit(*(current + 1)))) {
    CLEANUP_IF_FAIL(set_token_number(token, current, nl),
                    "Couldn't set token to number.");
    (*pos) += strlen(token->value);
    return 1;
  }

  // Word (instruction, register, keyword, or identifier)
  if (isalpha(*current) || *current == '.') {
    CLEANUP_IF_FAIL(
        set_token_word(token, current, nl),
        "Couldn't set token to instruction, register, keyword or identifier.");
    (*pos) += strlen(token->value);
    return 1;
  }

  // Unknown character
  CLEANUP_IF_FAIL(set_token_len(token, TOKEN_UNKNOWN, current, nl, 1),
                  "Couldn't set token to UNKNOWN.");
  (*pos)++;
  return 1;

cleanup:
  (*pos)++; // not to endup in an infinite loop...
  return 0;
}

// TODO: continue here

int set_token(struct Token *token, const enum Token_Type type,
              const char *value, const size_t nl) {
  size_t val_len = 0;

  CLEANUP_IF_FAIL(token, "TODO:");

  token->type = type;
  token->line_number = nl;
  if (!value) { // allow NULL in value
    return 1;
  }

  val_len = strlen(value);
  CLEANUP_IF_FAIL(val_len + 1 <= TOKEN_MAX_VALUE_LEN, "TODO:");

  memcpy(token->value, value, val_len);
  token->value[val_len] = '\0';

  return 1;

cleanup:
  return 0;
}

int set_token_len(struct Token *token, const enum Token_Type type,
                  const char *value, const size_t nl, const size_t len) {
  CLEANUP_IF_FAIL(token && value && len > 0 && len + 1 <= TOKEN_MAX_VALUE_LEN,
                  "TODO:");

  token->type = type;
  token->line_number = nl;
  memcpy(token->value, value, len);
  token->value[len] = '\0';

  return 1;

cleanup:
  return 0;
}

int set_token_string(struct Token *token, const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  CLEANUP_IF_FAIL(token && s, "TODO:");

  while (*curr && *curr != '"') {
    n_chars++;
    curr++;
  }
  CLEANUP_IF_FAIL(*curr, "TODO:"); // strings end wasnt reached due to '\0'

  CLEANUP_IF_FAIL(set_token_len(token, TOKEN_STRING, s, nl, n_chars), "TODO:");

  return 1;

cleanup:
  return 0;
}

int set_token_label(struct Token *token, const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  CLEANUP_IF_FAIL(token && s && *s == '@', "TODO:");
  n_chars++; // the @ at the beginning
  curr++;

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  CLEANUP_IF_FAIL(n_chars >= 2, "TODO:"); // too little characters

  CLEANUP_IF_FAIL(set_token_len(token, TOKEN_LABEL, s, nl, n_chars), "TODO:");

  return 1;

cleanup:
  return 0;
}

int set_token_number(struct Token *token, const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  CLEANUP_IF_FAIL(token && s, "TODO:");

  if (*curr == '-') { // optional negative number
    n_chars++;
    curr++;
  }

  while (isdigit(*curr)) {
    n_chars++;
    curr++;
  }

  CLEANUP_IF_FAIL((n_chars > 1 && *s == '-') || (n_chars > 0),
                  "TODO:"); // too little chars

  CLEANUP_IF_FAIL(set_token_len(token, TOKEN_NUMBER, s, nl, n_chars), "TODO:");

  return 1;

cleanup:
  return 0;
}

int set_token_word(struct Token *token, const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  enum Token_Type type = TOKEN_UNKNOWN;
  CLEANUP_IF_FAIL(token && s, "TODO:");

  if (*curr && *curr == '.') {
    n_chars++;
    curr++;
  }

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  type = lexer_classify_word(s, n_chars);
  CLEANUP_IF_FAIL(set_token_len(token, type, s, nl, n_chars), "TODO:");

  return 1;

cleanup:
  return 0;
}

enum Token_Type lexer_classify_word(const char *word, const size_t num) {
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
  if (instruction_is_mnemonic(word, num)) {
    return TOKEN_INSTRUCTION;
  }
  // SPECIALS
  IDENTIFY("DUP", TOKEN_DUP);
  IDENTIFY("OFFSET", TOKEN_OFFSET);

  return TOKEN_IDENTIFIER;
}
