// test_parser.c
// Simple unit tests for parser / parser_grammar public API.
// Compile example (adjust source file names as needed):
//   gcc -std=c11 -Wall -Wextra test_parser.c parser.c parser_grammar.c lexer.c
//   -o test_parser
// then run: ./test_parser

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../src/lexer.h"
#include "../src/parser.h"
#include "../src/parser_grammar.h"

// Counters for simple test harness
static int tests_run = 0;
static int tests_passed = 0;

#define RUN_TEST(fn)                                                           \
  do {                                                                         \
    printf("RUN: %s\n", #fn);                                                  \
    tests_run++;                                                               \
    if (fn()) {                                                                \
      tests_passed++;                                                          \
      printf(" OK\n\n");                                                       \
    } else {                                                                   \
      printf(" FAIL\n\n");                                                     \
    }                                                                          \
  } while (0)

// Helper: allocate a token (caller must free)
static struct Token *make_token(enum Token_Type type, const char *value,
                                size_t line) {
  struct Token *t = malloc(sizeof(*t));
  if (!t)
    return NULL;
  t->type = type;
  t->line_number = line;
  if (value) {
    strncpy(t->value, value, TOKEN_MAX_VALUE_LEN - 1);
    t->value[TOKEN_MAX_VALUE_LEN - 1] = '\0';
  } else {
    t->value[0] = '\0';
  }
  return t;
}

// Free array of token pointers and tokens themselves
static void free_token_array(struct Token *arr[], size_t count) {
  for (size_t i = 0; i < count; ++i) {
    free(arr[i]);
  }
}

// Build NULL-terminated token pointer array (last must be EOF token).
// Returns number of tokens (including EOF) via out_count. Returned array is
// malloc'd and must be freed by caller (free()) but tokens themselves must be
// freed separately.
static struct Token **build_token_ptr_array(struct Token *tokens[], size_t n,
                                            size_t *out_count) {
  // make array of pointers sized n+1 (for EOF) and final NULL sentinel not
  // required by APIs, but we keep explicit EOF token as last element.
  struct Token **arr = malloc((n + 1) * sizeof(struct Token *));
  if (!arr)
    return NULL;
  for (size_t i = 0; i < n; ++i)
    arr[i] = tokens[i];
  // caller should ensure tokens[n-1] is EOF if they intended so; we don't
  // insert extra EOF here.
  *out_count = n;
  return arr;
}

/*
 * Tests
 */

// 1) test create + free for a few statement types
static int test_p_stmt_create_and_free(void) {
  struct Parsed_Statement *ps;

  ps = p_stmt_create(STMT_KMA, 123);
  if (!ps) {
    printf("test_p_stmt_create_and_free_*1");
    return 0;
  }
  if (ps->type != STMT_KMA) {
    p_stmt_free(&ps);
    printf("test_p_stmt_create_and_free_*2");
    return 0;
  }
  if (ps->line_number != 123) {
    p_stmt_free(&ps);
    printf("test_p_stmt_create_and_free_*3");
    return 0;
  }
  p_stmt_free(&ps);
  if (ps != NULL) {
    printf("test_p_stmt_create_and_free_*4");
    return 0; // should set pointer to NULL
  }
  // label def
  ps = p_stmt_create(STMT_LABEL_DEF, 7);
  if (!ps) {
    printf("test_p_stmt_create_and_free_*5");
    return 0;
  }
  if (ps->type != STMT_LABEL_DEF) {
    p_stmt_free(&ps);
    printf("test_p_stmt_create_and_free_*6");
    return 0;
  }
  p_stmt_free(&ps);
  if (ps != NULL) {
    printf("test_p_stmt_create_and_free_*7");
    return 0;
  }
  return 1;
}

// 2) test p_stmt_init / p_stmt_deinit on stack-allocated memory
static int test_p_stmt_init_deinit(void) {
  struct Parsed_Statement *ps = malloc(sizeof(*ps));
  if (!ps)
    return 0;
  memset(ps, 0, sizeof(*ps));

  // initialize as instruction
  if (!p_stmt_init(ps, STMT_INSTRUCTION, 42)) {
    free(ps);
    return 0;
  }
  if (ps->type != STMT_INSTRUCTION) {
    p_stmt_deinit(ps);
    free(ps);
    return 0;
  }
  // deinit (should not crash)
  p_stmt_deinit(ps);

  // initialize as data declaration
  if (!p_stmt_init(ps, STMT_DATA_DECL, 13)) {
    free(ps);
    return 0;
  }
  if (ps->type != STMT_DATA_DECL) {
    p_stmt_deinit(ps);
    free(ps);
    return 0;
  }
  p_stmt_deinit(ps);

  free(ps);
  return 1;
}

// 3) grammar_line_kma: TOKEN_KMA, TOKEN_EOF -> GRM_MATCH and pstmt->type ==
// STMT_KMA
static int test_grammar_kma(void) {
  struct Token *t1 = make_token(TOKEN_KMA, "KMA", 1);
  struct Token *t2 = make_token(TOKEN_EOF, "", 1);
  if (!t1 || !t2) {
    free(t1);
    free(t2);
    return 0;
  }

  struct Token *tokens[] = {t1, t2};
  size_t tn;
  struct Token **parr = build_token_ptr_array(tokens, 2, &tn);
  if (!parr) {
    free_token_array(tokens, 2);
    return 0;
  }

  struct Parsed_Statement *pstmt = p_stmt_create(STMT_NONE, 1);
  if (!pstmt) {
    free(parr);
    free_token_array(tokens, 2);
    return 0;
  }

  enum Err_Grm res = grammar_line_kma(pstmt, (const struct Token **)parr);
  int ok = (res == GRM_MATCH) && (pstmt->type == STMT_KMA);

  p_stmt_free(&pstmt);
  free(parr);
  free_token_array(tokens, 2);
  return ok;
}

// 4) grammar_line_code: TOKEN_SECTION_CODE, TOKEN_EOF -> GRM_MATCH =>
// STMT_SECTION_CODE
static int test_grammar_code(void) {
  struct Token *t1 = make_token(TOKEN_SECTION_CODE, ".CODE", 2);
  struct Token *t2 = make_token(TOKEN_EOF, "", 2);
  if (!t1 || !t2) {
    free(t1);
    free(t2);
    return 0;
  }

  struct Token *tokens[] = {t1, t2};
  size_t tn;
  struct Token **parr = build_token_ptr_array(tokens, 2, &tn);
  if (!parr) {
    free_token_array(tokens, 2);
    return 0;
  }

  struct Parsed_Statement *pstmt = p_stmt_create(STMT_NONE, 2);
  if (!pstmt) {
    free(parr);
    free_token_array(tokens, 2);
    return 0;
  }

  enum Err_Grm res = grammar_line_code(pstmt, (const struct Token **)parr);
  int ok = (res == GRM_MATCH) && (pstmt->type == STMT_SECTION_CODE);

  p_stmt_free(&pstmt);
  free(parr);
  free_token_array(tokens, 2);
  return ok;
}

// 5) grammar_line_data: TOKEN_SECTION_DATA, TOKEN_EOF -> GRM_MATCH =>
// STMT_SECTION_DATA
static int test_grammar_data(void) {
  struct Token *t1 = make_token(TOKEN_SECTION_DATA, ".DATA", 3);
  struct Token *t2 = make_token(TOKEN_EOF, "", 3);
  if (!t1 || !t2) {
    free(t1);
    free(t2);
    return 0;
  }

  struct Token *tokens[] = {t1, t2};
  size_t tn;
  struct Token **parr = build_token_ptr_array(tokens, 2, &tn);
  if (!parr) {
    free_token_array(tokens, 2);
    return 0;
  }

  struct Parsed_Statement *pstmt = p_stmt_create(STMT_NONE, 3);
  if (!pstmt) {
    free(parr);
    free_token_array(tokens, 2);
    return 0;
  }

  enum Err_Grm res = grammar_line_data(pstmt, (const struct Token **)parr);
  int ok = (res == GRM_MATCH) && (pstmt->type == STMT_SECTION_DATA);

  p_stmt_free(&pstmt);
  free(parr);
  free_token_array(tokens, 2);
  return ok;
}

// 6) grammar_line_label: TOKEN_LABEL, TOKEN_EOF -> GRM_MATCH => STMT_LABEL_DEF
static int test_grammar_label(void) {
  // label name should include @ by your header's convention; use "@L1"
  struct Token *t1 = make_token(TOKEN_LABEL, "@L1", 4);
  struct Token *t2 = make_token(TOKEN_EOF, "", 4);
  if (!t1 || !t2) {
    free(t1);
    free(t2);
    return 0;
  }

  struct Token *tokens[] = {t1, t2};
  size_t tn;
  struct Token **parr = build_token_ptr_array(tokens, 2, &tn);
  if (!parr) {
    free_token_array(tokens, 2);
    return 0;
  }

  struct Parsed_Statement *pstmt = p_stmt_create(STMT_NONE, 4);
  if (!pstmt) {
    free(parr);
    free_token_array(tokens, 2);
    return 0;
  }

  enum Err_Grm res = grammar_line_label(pstmt, (const struct Token **)parr);
  int ok = (res == GRM_MATCH) && (pstmt->type == STMT_LABEL_DEF);

  // additionally, if label name present, check it copied
  if (ok) {
    if (strlen(pstmt->content.label_def.label_name) == 0)
      ok = 0;
    else {
      // label name in header _includes_ @; check substring
      if (strstr(pstmt->content.label_def.label_name, "L1") == NULL)
        ok = 0;
    }
  }

  p_stmt_free(&pstmt);
  free(parr);
  free_token_array(tokens, 2);
  return ok;
}

// Aggregator test: run all above tests
int main(void) {
  printf("Parser / Grammar minimal tests\n");
  printf("===============================\n\n");

  RUN_TEST(test_p_stmt_create_and_free);
  RUN_TEST(test_p_stmt_init_deinit);
  RUN_TEST(test_grammar_kma);
  RUN_TEST(test_grammar_code);
  RUN_TEST(test_grammar_data);
  RUN_TEST(test_grammar_label);

  printf("Summary: %d/%d tests passed\n", tests_passed, tests_run);
  return (tests_passed == tests_run) ? EXIT_SUCCESS : EXIT_FAILURE;
}
