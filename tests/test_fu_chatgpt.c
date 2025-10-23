#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../src/fileutil.h"

static void print_header(const char *name) { printf("\n=== %s ===\n", name); }

// Helpers
static const char *TMP_FILE = "tmp_fu_test_file.txt";
static const char *TMP_DIR = "tmp_fu_test_dir";
static const char *NON_EXISTENT = "tmp_fu_nonexistent_path.txt";

// Creates a file with some content
static void create_file(const char *path) {
  FILE *f = fopen(path, "w");
  assert(f && "Failed to create test file");
  fputs("temporary test content", f);
  fclose(f);
}

// Creates a directory if not exists
static void create_dir(const char *path) {
  struct stat st;
  if (stat(path, &st) == -1) {
    int r = mkdir(path, 0700);
    assert(r == 0 && "Failed to create test directory");
  }
}

// Clean up temp file/dir
static void cleanup(void) {
  unlink(TMP_FILE);
  rmdir(TMP_DIR);
}

// --- TESTS ---

static void test_path_exists(void) {
  print_header("test_path_exists");

  cleanup();
  create_file(TMP_FILE);
  create_dir(TMP_DIR);

  assert(fu_path_exists(TMP_FILE) == 1);
  assert(fu_path_exists(TMP_DIR) == 1);
  assert(fu_path_exists(NON_EXISTENT) == 0);

  printf("✅ path_exists passed.\n");
}

static void test_is_file_and_dir(void) {
  print_header("test_is_file_and_dir");

  cleanup();
  create_file(TMP_FILE);
  create_dir(TMP_DIR);

  assert(fu_is_file(TMP_FILE) == 1);
  assert(fu_is_dir(TMP_FILE) == 0);
  assert(fu_is_dir(TMP_DIR) == 1);
  assert(fu_is_file(TMP_DIR) == 0);

  printf("✅ is_file_and_dir passed.\n");
}

static void test_can_read(void) {
  print_header("test_can_read");

  cleanup();
  create_file(TMP_FILE);

  assert(fu_can_read(TMP_FILE) == 1);
  assert(fu_can_read(NON_EXISTENT) == 0);

  printf("✅ can_read passed.\n");
}

static void test_can_write_file(void) {
  print_header("test_can_write_file");

  cleanup();
  create_file(TMP_FILE);

  assert(fu_can_write(TMP_FILE) == 1);
  assert(fu_can_write_file(TMP_FILE) == 1);
  assert(fu_can_write_dir(TMP_FILE) == 0);

  printf("✅ can_write_file passed.\n");
}

static void test_can_write_dir(void) {
  print_header("test_can_write_dir");

  cleanup();
  create_dir(TMP_DIR);

  assert(fu_can_write(TMP_DIR) == 1);
  assert(fu_can_write_dir(TMP_DIR) == 1);
  assert(fu_can_write_file(TMP_DIR) == 0);

  printf("✅ can_write_dir passed.\n");
}

static void test_can_write_parent_dir(void) {
  print_header("test_can_write_parent_dir");

  cleanup();
  create_dir(TMP_DIR);

  char subpath[256];
  snprintf(subpath, sizeof(subpath), "%s/subfile.txt", TMP_DIR);

  assert(fu_can_write_parent_dir(subpath) == 1);
  assert(fu_can_write_parent_dir(NON_EXISTENT) == 1);

  printf("✅ can_write_parent_dir passed.\n");
}

static void test_invalid_inputs(void) {
  print_header("test_invalid_inputs");

  // NULL pointers should not crash
  fu_path_exists(NULL);
  fu_is_file(NULL);
  fu_is_dir(NULL);
  fu_can_read(NULL);
  fu_can_write(NULL);
  fu_can_write_file(NULL);
  fu_can_write_dir(NULL);
  fu_can_write_parent_dir(NULL);

  printf("✅ invalid_inputs (no crash) passed.\n");
}

// --- MAIN ---
int main(void) {
  printf("Running File Util tests...\n");

  test_path_exists();
  test_is_file_and_dir();
  test_can_read();
  test_can_write_file();
  test_can_write_dir();
  test_can_write_parent_dir();
  test_invalid_inputs();

  cleanup();
  printf("\nAll File Util tests passed successfully.\n");
  return 0;
}
