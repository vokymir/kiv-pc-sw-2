#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "instruction.h"
#include "lexer.h"
#include "memory.h"

#define TOKENS_INITIAL_CAPACITY 16
#define TOKENS_CAPACITY_MULT 2

// ===== STRUCTS =====

struct Token_Arr {
  struct Token *tokens;
  size_t count;
  size_t capacity;
};

// ===== MACROS =====

#define IDENTIFY(ch, type)                                                     \
  if (num == sizeof(ch) - 1 && strncmp(word, (ch), num) == 0) {                \
    return (type);                                                             \
  }

// ===== PRIVATE FUNCTION DECLARATIONS  =====

static const char *token_type_to_str(enum Token_Type type);

// Initialize the insides of token array.
// Return 1 on success, 0 on failure.
static int _tkar_init(struct Token_Arr *arr);

// Free all insides of token array.
static void _tkar_deinit(struct Token_Arr *arr);

// Ensure that in the token array is enough space for additional tokens.
// Return 1 on success, 0 on failure.
static int _tkar_ensure_capacity(struct Token_Arr *arr,
                                 size_t additional_tokens);

// Skip all whitespaces or comments in line by INCREMENTING the pos value.
// Return 1 if there is a token waiting to be parsed on pos.
// Return 0 if end of line was reached.
static int _lexer_skip_to_next_token(const char *line, const size_t len,
                                     size_t *pos);

// Based on current position in the line, update the given token.
// Update pos to one char after token characters.
// Return 1 on success, 0 on failure.
static int _lexer_set_next_token(struct Token *token, const char *line,
                                 const size_t len, size_t *pos,
                                 const size_t nl);

// Update token to have given parameters.
// Value can be NULL, in that case a token->value is irrelevant.
// Return 1 on success, 0 on failure.
static int _lexer_set_token(struct Token *token, const enum Token_Type type,
                            const char *value, const size_t nl);

// Update token to have given parameters.
// DOESN'T SUPPORT 0-length value!
// Only uses first len characters from given value = good for non-NULL
// terminated strings. It terminates the token->value by itself! Set Return 1 on
// success, 0 on failure.
static int _lexer_set_token_len(struct Token *token, const enum Token_Type type,
                                const char *value, const size_t nl,
                                const size_t len);

// The string in DATA segments. Takes pointer AFTER first QUOTE.
// Update the given token to have all info.
// Return 1 on success, 0 on failure.
static int _lexer_set_token_string(struct Token *token, const char *s,
                                   const size_t nl);

// Take pointer to the '@'.
// Set token to updated values.
// Return 1 on success, 0 on failure.
static int _lexer_set_token_label(struct Token *token, const char *s,
                                  const size_t nl);

// Take pointer to first digit (or minus sign).
// Set token for static number, found in code.
// Return 1 on success, 0 on failure.
static int _lexer_set_token_number(struct Token *token, const char *s,
                                   const size_t nl);

// Take pointer to first letter.
// Distinguish between different words and set token to the one which it is.
// Return 1 on success, 0 on failure.
static int _lexer_set_token_word(struct Token *token, const char *s,
                                 const size_t nl);

// Classify the first <num> number of characters of word.
// Return the adequate TokenType, or TOKEN_UNKNOWN if any error.
static enum Token_Type _lexer_classify_word(const char *word, const size_t num);

// ===== PUBLIC FUNCTIONS =====

struct Token *lexer_tokenize_line(const char *line, const size_t nl) {
  struct Token_Arr arr = {0};
  struct Token *token = NULL;
  size_t pos = 0;
  size_t len = 0;

  if (!line) {
    return NULL;
  }

  CLEANUP_IF_FAIL(_tkar_init(&arr));

  len = strlen(line);

  // Process the whole line
  while (_lexer_skip_to_next_token(line, len, &pos)) {
    CLEANUP_IF_FAIL(_tkar_ensure_capacity(&arr, 1));
    token = &arr.tokens[arr.count];

    CLEANUP_IF_FAIL(_lexer_set_next_token(token, line, len, &pos, nl));
    arr.count++;

    token = NULL;
  }

  // Add EOF to the end
  CLEANUP_IF_FAIL(_tkar_ensure_capacity(&arr, 1));
  token = &arr.tokens[arr.count];

  CLEANUP_IF_FAIL(_lexer_set_token(token, TOKEN_EOF, NULL, nl));
  arr.count++;

  return arr.tokens;

cleanup:
  _tkar_deinit(&arr);
  return NULL;
}

void lexer_free_tokens(struct Token *tokens) {
  if (tokens) {
    jree(tokens);
  }
  return;
}

void print_token(const struct Token *token) {
  if (!token) {
    printf("(null token)\n");
    return;
  }

  printf("Token{ type=%s, value=\"%s\", line=%zu }\n",
         token_type_to_str(token->type), token->value, token->line_number);
}

void print_tokens(const struct Token *tokens) {
  if (!tokens) {
    printf("(null token array)\n");
    return;
  }

  size_t i = 0;
  while (tokens[i].type != TOKEN_EOF) {
    printf("[%zu] ", i);
    print_token(&tokens[i]);
    i++;
  }
  printf("[%zu] Token{ type=EOF }\n", i);
}

// ===== PRIVATE FUNCTIONS =====

static const char *token_type_to_str(enum Token_Type type) {
  switch (type) {
  case TOKEN_INSTRUCTION:
    return "INSTRUCTION";
  case TOKEN_REGISTER:
    return "REGISTER";
  case TOKEN_NUMBER:
    return "NUMBER";
  case TOKEN_IDENTIFIER:
    return "IDENTIFIER";
  case TOKEN_LABEL:
    return "LABEL";
  case TOKEN_COMMA:
    return "COMMA";
  case TOKEN_SECTION_DATA:
    return "SECTION_DATA";
  case TOKEN_SECTION_CODE:
    return "SECTION_CODE";
  case TOKEN_KMA:
    return "KMA";
  case TOKEN_OFFSET:
    return "OFFSET";
  case TOKEN_QUESTION:
    return "QUESTION";
  case TOKEN_STRING:
    return "STRING";
  case TOKEN_DATA_TYPE:
    return "DATA_TYPE";
  case TOKEN_DUP:
    return "DUP";
  case TOKEN_LPAREN:
    return "LPAREN";
  case TOKEN_RPAREN:
    return "RPAREN";
  case TOKEN_EOF:
    return "EOF";
  case TOKEN_UNKNOWN:
  default:
    return "UNKNOWN";
  }
}

static int _tkar_init(struct Token_Arr *arr) {
  CLEANUP_IF_FAIL(arr);

  arr->tokens = jalloc(TOKENS_INITIAL_CAPACITY * sizeof(struct Token));
  CLEANUP_IF_FAIL(arr->tokens);

  arr->count = 0;
  arr->capacity = TOKENS_INITIAL_CAPACITY;

  return 1;

cleanup:
  return 0;
}

static void _tkar_deinit(struct Token_Arr *arr) {
  CLEANUP_IF_FAIL(arr);

  if (arr->tokens) {
    jree_clear((void **)&arr->tokens);
  }
  arr->count = 0;
  arr->capacity = 0;

cleanup:
  return;
}

static int _tkar_ensure_capacity(struct Token_Arr *arr,
                                 size_t additional_tokens) {
  size_t req = 0, new_cap = 0;
  struct Token *new_tokens = NULL;
  CLEANUP_IF_FAIL(arr);

  if (additional_tokens == 0) {
    return 1;
  }

  req = arr->count + additional_tokens;
  if (req <= arr->capacity) {
    return 1;
  }

  new_cap = arr->capacity ? arr->capacity : TOKENS_INITIAL_CAPACITY;
  while (new_cap < req) {
    new_cap *= TOKENS_CAPACITY_MULT;
  }

  new_tokens = jealloc(arr->tokens, new_cap * sizeof(struct Token));
  CLEANUP_IF_FAIL(new_tokens);

  arr->tokens = new_tokens;
  arr->capacity = new_cap;
  return 1;

cleanup:
  return 0;
}

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

static int _lexer_set_next_token(struct Token *token, const char *line,
                                 const size_t len, size_t *pos,
                                 const size_t nl) {
  const char *current = NULL;
  CLEANUP_IF_FAIL(token && line && pos && len > 0 && len > *pos);

  current = &line[*pos];

  // Single-character tokens
  if (*current == ',') {
    (*pos)++;
    return _lexer_set_token(token, TOKEN_COMMA, ",", nl);
  }

  if (*current == '(') {
    (*pos)++;
    return _lexer_set_token(token, TOKEN_LPAREN, "(", nl);
  }

  if (*current == ')') {
    (*pos)++;
    return _lexer_set_token(token, TOKEN_RPAREN, ")", nl);
  }

  if (*current == '?') {
    (*pos)++;
    return _lexer_set_token(token, TOKEN_QUESTION, "?", nl);
  }

  // String literal in .DATA segment
  if (*current == '"') {
    CLEANUP_IF_FAIL(_lexer_set_token_string(token, current + 1,
                                            nl)); // +1 for opening quote
    (*pos) += strlen(token->value) + 2; // +2 for the quotes on begin/end
    return 1;
  }

  // Label (starts with @)
  if (*current == '@') {
    CLEANUP_IF_FAIL(_lexer_set_token_label(token, current, nl));
    (*pos) += strlen(token->value) + 1; // +1 for ':'
    return 1;
  }

  // Number (digit or negative number)
  if (isdigit(*current) ||
      (*current == '-' && *pos + 1 < len && isdigit(*(current + 1)))) {
    CLEANUP_IF_FAIL(_lexer_set_token_number(token, current, nl));
    (*pos) += strlen(token->value);
    return 1;
  }

  // Word (instruction, register, keyword, or identifier)
  if (isalpha(*current) || *current == '.') {
    CLEANUP_IF_FAIL(_lexer_set_token_word(token, current, nl));
    (*pos) += strlen(token->value);
    return 1;
  }

  // Unknown character
  CLEANUP_IF_FAIL(_lexer_set_token_len(token, TOKEN_UNKNOWN, current, nl, 1));
  (*pos)++;
  return 1;

cleanup:
  (*pos)++; // not to endup in infinity loop...
  return 0;
}

static int _lexer_set_token(struct Token *token, const enum Token_Type type,
                            const char *value, const size_t nl) {
  size_t val_len = 0;

  CLEANUP_IF_FAIL(token);

  token->type = type;
  token->line_number = nl;
  if (!value) { // allow NULL in value
    return 1;
  }

  val_len = strlen(value);
  CLEANUP_IF_FAIL(val_len + 1 <= TOKEN_MAX_VALUE_LEN);

  memcpy(token->value, value, val_len);
  token->value[val_len] = '\0';

  return 1;

cleanup:
  return 0;
}

static int _lexer_set_token_len(struct Token *token, const enum Token_Type type,
                                const char *value, const size_t nl,
                                const size_t len) {
  CLEANUP_IF_FAIL(token && value && len > 0 && len + 1 <= TOKEN_MAX_VALUE_LEN);

  token->type = type;
  token->line_number = nl;
  memcpy(token->value, value, len);
  token->value[len] = '\0';

  return 1;

cleanup:
  return 0;
}

static int _lexer_set_token_string(struct Token *token, const char *s,
                                   const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  CLEANUP_IF_FAIL(token && s);

  while (*curr && *curr != '"') {
    n_chars++;
    curr++;
  }
  CLEANUP_IF_FAIL(*curr); // strings end wasnt reached due to '\0'

  CLEANUP_IF_FAIL(_lexer_set_token_len(token, TOKEN_STRING, s, nl, n_chars));

  return 1;

cleanup:
  return 0;
}

static int _lexer_set_token_label(struct Token *token, const char *s,
                                  const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  CLEANUP_IF_FAIL(token && s && *s == '@');
  n_chars++; // the @ at the beginning
  curr++;

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  CLEANUP_IF_FAIL(n_chars >= 2); // too little characters

  CLEANUP_IF_FAIL(_lexer_set_token_len(token, TOKEN_LABEL, s, nl, n_chars));

  return 1;

cleanup:
  return 0;
}

static int _lexer_set_token_number(struct Token *token, const char *s,
                                   const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  CLEANUP_IF_FAIL(token && s);

  if (*curr == '-') { // optional negative number
    n_chars++;
    curr++;
  }

  while (isdigit(*curr)) {
    n_chars++;
    curr++;
  }

  CLEANUP_IF_FAIL((n_chars > 1 && *s == '-') ||
                  (n_chars > 0)); // too little chars

  CLEANUP_IF_FAIL(_lexer_set_token_len(token, TOKEN_NUMBER, s, nl, n_chars));

  return 1;

cleanup:
  return 0;
}

static int _lexer_set_token_word(struct Token *token, const char *s,
                                 const size_t nl) {
  size_t n_chars = 0;
  const char *curr = s;
  enum Token_Type type = TOKEN_UNKNOWN;
  CLEANUP_IF_FAIL(token && s);

  if (*curr && *curr == '.') {
    n_chars++;
    curr++;
  }

  while (*curr && (isalnum(*curr) || *curr == '_')) {
    n_chars++;
    curr++;
  }

  type = _lexer_classify_word(s, n_chars);
  CLEANUP_IF_FAIL(_lexer_set_token_len(token, type, s, nl, n_chars));

  return 1;

cleanup:
  return 0;
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
