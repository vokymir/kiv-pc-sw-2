/* tests/test_args_chatgpt.c
 *
 * C99-compatible unit tests for args module.
 * Define POSIX feature macro before includes so mkstemp/mkdtemp are declared.
 */

#define _POSIX_C_SOURCE 200809L

#include "../src/args.h"
#include "../src/common.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Adjust this if Err_Main uses a different success value */
#define EXPECT_OK ERR_NO_ERROR

/* Helper: create temporary file, return malloc'ed path (caller must free). File
 * remains on disk. */
static char *make_temp_file(void) {
  char tmpl[] = "/tmp/test_args_srcXXXXXX";
  int fd = mkstemp(tmpl);
  if (fd < 0) {
    return NULL;
  }
  close(fd);

  /* allocate and copy path */
  size_t len = strlen(tmpl) + 1;
  char *p = (char *)malloc(len);
  if (!p) {
    return NULL;
  }
  memcpy(p, tmpl, len);
  return p;
}

/* Helper: create temporary directory, return malloc'ed path (caller must free)
 */
static char *make_temp_dir(void) {
  char tmpl[] = "/tmp/test_args_dirXXXXXX";
  char *res = mkdtemp(tmpl);
  if (!res) {
    return NULL;
  }

  size_t len = strlen(res) + 1;
  char *p = (char *)malloc(len);
  if (!p) {
    return NULL;
  }
  memcpy(p, res, len);
  return p;
}

static void test_args_path_check_syntax(void) {
  printf("Testing args_path_check_syntax...\n");

  /* NULL pointer -> ARGS_INVALID_POINTER expected */
  enum Err_Args r_null = args_path_check_syntax(NULL, NULL, NULL);
  assert(r_null == ARGS_INVALID_POINTER);

  /* Empty string -> ARGS_PATH_EMPTY */
  enum Err_Args r_empty = args_path_check_syntax("", NULL, NULL);
  assert(r_empty == ARGS_PATH_EMPTY);

  /* Bad extension (suffix mismatch) */
  enum Err_Args r_bad_ext = args_path_check_syntax("file.txt", NULL, ".bin");
  assert(r_bad_ext == ARGS_PATH_BAD_EXTENSION);

  /* Bad prefix */
  enum Err_Args r_bad_pref = args_path_check_syntax(
      "/must/be/prefix/file.bin", "/must/be/a/different/prefix", ".bin");
  assert(r_bad_pref == ARGS_PATH_BAD_PREFIX);

  /* Valid path with required suffix */
  enum Err_Args r_ok = args_path_check_syntax("archive.bin", NULL, ".bin");
  assert(r_ok == ARGS_NO_ERROR);

  printf("  PASSED\n");
}

static void test_args_config_init_deinit_and_semantic(void) {
  printf("Testing args_config_init, args_config_deinit and "
         "args_path_check_semantic...\n");

  char *src = make_temp_file();
  assert(src != NULL);

  char *dir = make_temp_dir();
  assert(dir != NULL);

  /* Build target path inside created directory (parent writable) */
  size_t tlen = strlen(dir) + 1 + 16;
  char *target = (char *)malloc(tlen);
  assert(target != NULL);
  snprintf(target, tlen, "%s/%s", dir, "out.bin");

  /* Initialize zeroed Config */
  struct Config cfg;
  memset(&cfg, 0, sizeof(cfg));

  /* args_config_init should return 1 on success */
  int init_ok =
      args_config_init(&cfg, src, target, /*verbose*/ 1, /*instruction*/ 0);
  assert(init_ok == 1);

  /* Semantic check — expect Err_Main success (assumed 0 here) */
  int sem = args_path_check_semantic(&cfg);
  assert(sem == EXPECT_OK);

  /* Deinitialize and free internals */
  args_config_deinit(&cfg);

  /* Many implementations support safe repeated deinit; call again to check
   * safety */
  args_config_deinit(&cfg);

  /* Cleanup filesystem artifacts */
  unlink(src);
  free(src);
  free(target);
  rmdir(dir);
  free(dir);

  printf("  PASSED\n");
}

static void test_args_parse_minimal(void) {
  printf("Testing args_parse (minimal argv containing only program name)...\n");

  /* argv with only program name, argc = 1. Header states argv[0] is executable
   * name. */
  const char *argv1[] = {"prog"};
  struct Config cfg;
  memset(&cfg, 0, sizeof(cfg));

  enum Err_Main r = args_parse(&cfg, 1, argv1);
  assert(r != EXPECT_OK);

  args_config_deinit(&cfg);

  printf("  PASSED\n");
}

int main(void) {
  printf("\n=== Running args module tests ===\n\n");

  test_args_path_check_syntax();
  test_args_config_init_deinit_and_semantic();
  test_args_parse_minimal();

  printf("\n=== All args tests passed ===\n\n");
  return 0;
}
