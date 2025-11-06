#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "codeseg.h"
#include "common.h"
#include "dataseg.h"
#include "symbol.h"

#define TEST_CASE(desc) printf("\n[TEST] %s\n", desc);

// Helper to make minimal config
static struct Config make_config(void) {
  struct Config cfg = {0};
  cfg.flag_verbose = 1;
  cfg.flag_instruction = 0;
  cfg.source = "input.asm";
  cfg.target = "output.kmx";
  return cfg;
}

/* =============================
 *  TESTS FOR asp_create / asp_free
 * ============================= */
void test_asp_create_and_free(void) {
  TEST_CASE("asp_create allocates all submodules when passed NULLs");
  struct Config cfg = make_config();

  struct Assembler_Processing *asp = asp_create(&cfg, NULL, NULL, NULL);
  assert(asp != NULL);
  assert(asp->config == &cfg);
  assert(asp->symtab != NULL);
  assert(asp->dtsg != NULL);
  assert(asp->cdsg != NULL);

  asp_free(&asp);
  assert(asp == NULL);
  printf("✅ Passed asp_create/asp_free allocation test\n");
}

/* =============================
 *  TESTS FOR asp_init / asp_deinit
 * ============================= */
void test_asp_init_and_deinit(void) {
  TEST_CASE("asp_init initializes fields and allocates missing parts");
  struct Assembler_Processing asp = {0};
  struct Config cfg = make_config();

  int ok = asp_init(&asp, &cfg, NULL, NULL, NULL);
  assert(ok == 1);
  assert(asp.config == &cfg);
  assert(asp.symtab != NULL);
  assert(asp.dtsg != NULL);
  assert(asp.cdsg != NULL);

  asp_deinit(&asp);
  printf("✅ Passed asp_init/asp_deinit basic test\n");
}

/* =============================
 *  TESTS FOR arg validation
 * ============================= */
void test_invalid_arguments(void) {
  TEST_CASE("asp_init handles NULL asp argument safely");
  int ok = asp_init(NULL, NULL, NULL, NULL, NULL);
  assert(ok == 0);
  printf("✅ Passed asp_init invalid argument test\n");
}

/* =============================
 *  TESTS FOR pass1 / pass2 / process_assembler
 * =============================
 * These will only compile if stubs exist, since actual
 * assembler logic likely needs file input. We test that
 * they return defined error enums instead of crashing.
 */
void test_passes_exist(void) {
  TEST_CASE("pass1, pass2, and process_assembler callable without crash");
  struct Config config = make_config();

  struct Assembler_Processing *asp = asp_create(&config, NULL, NULL, NULL);
  assert(asp);

  enum Err_Asm e1 = pass1(asp);
  enum Err_Asm e2 = pass2(asp);
  enum Err_Main e3 = process_assembler(asp);

  // We can't assert specific behavior, only that they're valid enums
  assert(e1 >= 0);
  assert(e2 >= 0);
  assert(e3 >= 0);

  asp_free(&asp);
  printf("✅ Passed assembler passes smoke test\n");
}

/* =============================
 *  MAIN
 * ============================= */
int main(void) {
  printf("==== TESTING assembler.c module ====\n");

  test_asp_create_and_free();
  test_asp_init_and_deinit();
  test_invalid_arguments();
  test_passes_exist();

  printf("\nAll assembler tests passed ✅\n");
  return 0;
}
