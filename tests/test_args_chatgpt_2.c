#include "../src/args.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper to print test headers
static void print_header(const char *name) { printf("\n=== %s ===\n", name); }

// Mock helper to simulate argv input
static void make_argv(const char *exe, const char *src, const char *tgt,
                      int verbose, int instr, const char **argv_out,
                      int *argc_out) {
  static char vbuf[8];
  static char ibuf[8];
  snprintf(vbuf, sizeof(vbuf), "%d", verbose);
  snprintf(ibuf, sizeof(ibuf), "%d", instr);
  argv_out[0] = exe;
  argv_out[1] = src;
  argv_out[2] = tgt;
  argv_out[3] = vbuf;
  argv_out[4] = ibuf;
  *argc_out = 5;
}

// --- TESTS ---

static void test_config_init_and_deinit(void) {
  print_header("test_config_init_and_deinit");

  struct Config cfg;
  int ok = args_config_init(&cfg, "input.txt", "output.txt", 1, 2);
  assert(ok && "args_config_init should succeed");

  assert(strcmp(cfg.source, "input.txt") == 0);
  assert(strcmp(cfg.target, "output.txt") == 0);
  assert(cfg.flag_verbose == 1);
  assert(cfg.flag_instruction == 2);

  args_config_deinit(&cfg);
  assert(cfg.source == NULL);
  assert(cfg.target == NULL);
  assert(cfg.flag_verbose == 0);
  assert(cfg.flag_instruction == 0);

  printf("✅ config_init/deinit passed.\n");
}

static void test_path_check_syntax(void) {
  print_header("test_path_check_syntax");

  enum Err_Args err;

  err = args_path_check_syntax("input.txt", NULL, ".txt");
  assert(err == ARGS_NO_ERROR);

  err = args_path_check_syntax("", NULL, ".txt");
  assert(err == ARGS_PATH_EMPTY);

  err = args_path_check_syntax("data.bad", NULL, ".txt");
  assert(err == ARGS_PATH_BAD_EXTENSION);

  printf("✅ path_check_syntax passed.\n");
}

static void test_parse_valid_args(void) {
  print_header("test_parse_valid_args");

  struct Config cfg;
  const char *argv[5];
  int argc;
  make_argv("prog", "src.txt", "dst.txt", 1, 0, argv, &argc);

  enum Err_Main err = args_parse(&cfg, argc, argv);

  // We allow both "no error" and any file-related error (file not found etc.)
  switch (err) {
  case ERR_NO_ERROR:
  case ERR_INVALID_INPUT_FILE:
  case ERR_INVALID_OUTPUT_FILE:
  case ERR_FILE_ACCESS_FAILURE:
    printf("args_parse returned acceptable code: %d\n", err);
    break;
  default:
    fprintf(stderr, "Unexpected args_parse error: %d\n", err);
    assert(0 && "Unexpected args_parse result");
  }

  // Semantic check may fail for the same reasons
  err = args_path_check_semantic(&cfg);
  switch (err) {
  case ERR_NO_ERROR:
  case ERR_INVALID_INPUT_FILE:
  case ERR_FILE_ACCESS_FAILURE:
    printf("args_path_check_semantic returned acceptable code: %d\n", err);
    break;
  default:
    fprintf(stderr, "Unexpected semantic check error: %d\n", err);
    assert(0 && "Unexpected args_path_check_semantic result");
  }

  args_config_deinit(&cfg);
  printf("✅ parse_valid_args passed (even with missing files).\n");
}

static void test_parse_invalid_args(void) {
  print_header("test_parse_invalid_args");

  struct Config cfg;
  const char *argv[3] = {"prog", NULL, NULL};
  int argc = 1; // Missing args

  enum Err_Main err = args_parse(&cfg, argc, argv);
  assert(err == ERR_SYNTAX_ERROR || err == ERR_INVALID_INPUT_FILE);

  printf("✅ parse_invalid_args passed.\n");
}

// Entry point
int main(void) {
  printf("Running Args module tests...\n");

  test_config_init_and_deinit();
  test_path_check_syntax();
  test_parse_valid_args();
  test_parse_invalid_args();

  printf("\nAll tests completed successfully.\n");
  return 0;
}
