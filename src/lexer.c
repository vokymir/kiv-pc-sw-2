#include <ctype.h>
#include <stddef.h>
#include <string.h>

#include "common.h"
#include "instruction.h"
#include "lexer.h"
#include "llist.h"
#include "memory.h"

// ===== MACROS =====

#define IDENTIFY(ch, type)                                                     \
  if (num == sizeof(ch) - 1 && strncmp(word, (ch), num) == 0) {                \
    return (type);                                                             \
  }

// ===== PRIVATE FUNCTION DECLARATIONS  =====

// Skip all whitespaces or comments in line by INCREMENTING the pos value.
// Return 1 if there is a token waiting to be parsed on pos.
// Return 0 if end of line was reached.
static int _lexer_skip_to_next_token(const char *line, const size_t len,
                                     size_t *pos);

// Create next token, starting on pos.
// Update pos to one char after token characters.
// Return Token on success, NULL on failure.
static struct Token *_lexer_create_next_token(const char *line,
                                              const size_t len, size_t *pos,
                                              const size_t nl);

// Add a token to the list.
// Return 1 on success, 0 on failure.
// On failure, the token IS FREED.
static int _lexer_add_token_to_list(struct Llist *tokens_list,
                                    struct Token *token);

// Create token with given parameters, memcpy value using strlen().
// Return pointer to token on success, NULL on failure.
// Caller must free.
static struct Token *_lexer_create_token(const enum Token_Type type,
                                         const char *value, const size_t nl);

// Create token with given parameters, memcpy the value using len. If you want
// to end it eg by \0, you must pass (real-len+1) and manually rewrite it.
// Return pointer to token on success, NULL on failure. Caller must free.
static struct Token *_lexer_create_token_n(const enum Token_Type type,
                                           const char *value, const size_t nl,
                                           const size_t len);

// The string in DATA segments. Takes pointer AFTER first QUOTE.
// Return pointer to token on success, NULL on failure.
// Caller must free.
static struct Token *_lexer_create_token_string(const char *s, const size_t nl);

// Create token from pointer to the start of label (starting with @).
// Take pointer to the '@'.
// Return pointer to Token or NULL.
// Caller must free.
static struct Token *_lexer_create_token_label(const char *s, const size_t nl);

// Create token for static number, found wherever in code.
// Take pointer to first digit (or minus sign).
// Return pointer to Token or NULL.
// Caller must free.
static struct Token *_lexer_create_token_number(const char *s, const size_t nl);

// Distinguish between different words and return which one it is.
// Take pointer to first letter.
// Return pointer to Token or NULL.
// Caller must free.
static struct Token *_lexer_create_token_word(const char *s, const size_t nl);

// Classify the first <num> number of characters of word.
// Return the adequate TokenType, or TOKEN_UNKNOWN if any error.
static enum Token_Type _lexer_classify_word(const char *word, const size_t num);

// ===== PUBLIC FUNCTIONS =====

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
  CLEANUP_IF_FAIL(tokens);

  len = strlen(line);

  // Process the whole line
  while (_lexer_skip_to_next_token(line, len, &pos)) {
    token = _lexer_create_next_token(line, len, &pos, nl);

    CLEANUP_IF_FAIL(_lexer_add_token_to_list(tokens, token));
    token = NULL;
  }

  // Add EOF to the end
  token = _lexer_create_token(TOKEN_EOF, "", nl);
  CLEANUP_IF_FAIL(_lexer_add_token_to_list(tokens, token));

  return tokens;

cleanup:
  if (tokens) {
    llist_free(tokens, func);
  }
  return NULL;
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

// ===== PRIVATE FUNCTIONS =====

static int _lexer_skip_to_next_token(const char *line, const size_t len,
                                     size_t *pos) {
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

static struct Token *_lexer_create_next_token(const char *line,
                                              const size_t len, size_t *pos,
                                              const size_t nl) {
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

static int _lexer_add_token_to_list(struct Llist *tokens_list,
                                    struct Token *token) {
  if (!tokens_list || !token) {
    return 0;
  }

  CLEANUP_IF_FAIL(token->value);

  CLEANUP_IF_FAIL(llist_add(tokens_list, (void **)&token));

  return 1;

cleanup:
  lexer_free_token(token);
  return 0;
}

static struct Token *_lexer_create_token(const enum Token_Type type,
                                         const char *value, const size_t nl) {
  size_t val_len = strlen(value) + 1;
  struct Token *token = jalloc(sizeof(struct Token));

  CLEANUP_IF_FAIL(token);

  token->value = jalloc(sizeof(char) * val_len);
  CLEANUP_IF_FAIL(token->value);

  token->type = type;
  token->line_number = nl;
  memcpy(token->value, value, val_len);

  return token;

cleanup:
  if (token) {
    jree(token);
  }
  return NULL;
}

static struct Token *_lexer_create_token_n(const enum Token_Type type,
                                           const char *value, const size_t nl,
                                           const size_t len) {
  struct Token *token = jalloc(sizeof(struct Token));
  CLEANUP_IF_FAIL(token);

  token->value = jalloc(sizeof(char) * len);
  CLEANUP_IF_FAIL(token->value);

  token->type = type;
  token->line_number = nl;
  memcpy(token->value, value, len);

  return token;

cleanup:
  if (token) {
    jree(token);
  }
  return NULL;
}

static struct Token *_lexer_create_token_string(const char *s,
                                                const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  CLEANUP_IF_FAIL(s);

  while (*curr && *curr != '"') {
    n_chars++;
    curr++;
  }
  if (*curr == '\0') {
    return NULL; // unterminated string
  }

  token = _lexer_create_token_n(TOKEN_STRING, s, nl, n_chars + 1); // 1 for \0
  CLEANUP_IF_FAIL(token);
  token->value[n_chars] = '\0';

  return token;

cleanup:
  return NULL;
}

static struct Token *_lexer_create_token_label(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  CLEANUP_IF_FAIL(s && *s == '@');
  n_chars++; // the @ at the beginning
  curr++;

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  CLEANUP_IF_FAIL(n_chars >= 2); // too little characters

  token = _lexer_create_token_n(TOKEN_LABEL, s, nl, n_chars + 1); // 1 for \0
  CLEANUP_IF_FAIL(token);
  token->value[n_chars] = '\0';

  return token;

cleanup:
  return NULL;
}

static struct Token *_lexer_create_token_number(const char *s,
                                                const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  CLEANUP_IF_FAIL(s);

  if (*curr == '-') { // optional negative number
    n_chars++;
    curr++;
  }

  while (isdigit(*curr)) {
    n_chars++;
    curr++;
  }

  token = _lexer_create_token_n(TOKEN_NUMBER, s, nl, n_chars + 1); // 1 for \0
  CLEANUP_IF_FAIL(token);
  token->value[n_chars] = '\0';

  return token;

cleanup:
  return NULL;
}

static struct Token *_lexer_create_token_word(const char *s, const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  struct Token *token = NULL;
  enum Token_Type type = TOKEN_UNKNOWN;
  CLEANUP_IF_FAIL(s);

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
  CLEANUP_IF_FAIL(token);
  token->value[n_chars] = '\0';

  return token;

cleanup:
  return NULL;
}

static enum Token_Type _lexer_classify_word(const char *word,
                                            const size_t num) {
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
