// tests/test_codeseg_robust.c
#include "../src/codeseg.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void dump(const uint8_t *b, size_t n) {
  for (size_t i = 0; i < n; ++i)
    printf("%02X ", b[i]);
  printf("\n");
}

/* 1) create/free basic smoke test */
static void test_create_free(void) {
  printf("test_create_free...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);
  assert(cdsg_get_size(cd) == 0);
  const uint8_t *bytes = cdsg_get_bytes(cd);
  assert(bytes != NULL); /* bytes buffer should exist (capacity >= initial) */
  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

/* 2) append single bytes + bulk append (including count==0) */
static void test_append_byte_and_bs(void) {
  printf("test_append_byte_and_bs...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  /* append single bytes */
  assert(cdsg_app_b(cd, 0xAA) == 1);
  assert(cdsg_app_b(cd, 0xBB) == 1);
  assert(cdsg_get_size(cd) == 2);

  /* append buffer normally */
  uint8_t buf[] = {0x01, 0x02, 0x03};
  assert(cdsg_app_bs(cd, buf, sizeof(buf)) == 1);
  assert(cdsg_get_size(cd) == 2 + sizeof(buf));

  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[0] == 0xAA && b[1] == 0xBB);
  for (size_t i = 0; i < sizeof(buf); ++i)
    assert(b[2 + i] == buf[i]);

  /* append zero bytes should succeed and be a no-op */
  size_t size_before = cdsg_get_size(cd);
  assert(cdsg_app_bs(cd, buf, 0) == 1);
  assert(cdsg_get_size(cd) == size_before);

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

/* 3) append DWORD (uint32_t) little-endian correctness including boundary
 * values */
static void test_append_dw_endianness(void) {
  printf("test_append_dw_endianness...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  /* append a known dword */
  uint32_t dw = 0x11223344u;
  assert(cdsg_app_dw(cd, dw) == 1);
  assert(cdsg_get_size(cd) == 4);
  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[0] == 0x44 && b[1] == 0x33 && b[2] == 0x22 && b[3] == 0x11);

  /* append max dword 0xFFFFFFFF */
  assert(cdsg_app_dw(cd, 0xFFFFFFFFu) == 1);
  assert(cdsg_get_size(cd) == 8);
  assert(b[4] == 0xFF && b[5] == 0xFF && b[6] == 0xFF && b[7] == 0xFF);

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

/* 4) reserve & patch: reserve some bytes, check returned pos, size growth, and
 * patching bytes */
static void test_reserve_and_patch(void) {
  printf("test_reserve_and_patch...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  size_t initial = cdsg_get_size(cd);
  size_t pos = cdsg_reserve(cd, 4);
  /* expected: pos == initial, and size increased by 4 */
  assert(pos != SIZE_MAX);
  assert(pos == initial);
  assert(cdsg_get_size(cd) >= initial + 4);

  /* patch reserved bytes (cast away const - this is allowed for tests;
     production API could provide a patch function if preferred) */
  uint8_t *mutable = (uint8_t *)cdsg_get_bytes(cd);
  mutable[pos + 0] = 0xDE;
  mutable[pos + 1] = 0xAD;
  mutable[pos + 2] = 0xBE;
  mutable[pos + 3] = 0xEF;

  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[pos + 0] == 0xDE && b[pos + 3] == 0xEF);

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

/* 5) reserve(0) should be a no-op returning current size (not an arbitrary
 * value) */
static void test_reserve_zero_noop(void) {
  printf("test_reserve_zero_noop...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  size_t before = cdsg_get_size(cd);
  size_t r = cdsg_reserve(cd, 0);
  /* Correct behavior: r == before (no-op). If implementation returns 1 always,
     this test will detect that. */
  assert(r == before);

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

/* 6) reserve huge / overflow detection
   Expectation: when asked to reserve absurdly large number the function should
   fail and return SIZE_MAX (failure sentinel). If it succeeds, it may cause
   overflow — test will fail. */
static void test_reserve_overflow_detection(void) {
  printf("test_reserve_overflow_detection...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  /* try to reserve an enormous chunk that should not be possible */
  size_t big = SIZE_MAX / 2;
  size_t pos = cdsg_reserve(cd, big);
  /* robust implementation should detect impossibility and return SIZE_MAX */
  assert(pos == SIZE_MAX);

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED (or the implementation failed this check)\n");
}

/* 7) growth & capacity: append many bytes and ensure capacity grows and bytes
 * preserved */
static void test_growth_and_capacity(void) {
  printf("test_growth_and_capacity...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  size_t initial_capacity = cd->capacity;
  size_t to_append = initial_capacity * 4 + 3; /* force several resizes */

  for (size_t i = 0; i < to_append; ++i) {
    assert(cdsg_app_b(cd, (uint8_t)(i & 0xFF)) == 1);
  }
  assert(cdsg_get_size(cd) == to_append);
  assert(cd->capacity >= cdsg_get_size(cd));

  /* verify some sample bytes for correctness */
  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[0] == 0);
  assert(b[1] == 1);
  assert(b[initial_capacity] == (uint8_t)(initial_capacity & 0xFF));

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

/* 8) null argument handling: functions should fail gracefully with NULL */
static void test_null_argument_handling(void) {
  printf("test_null_argument_handling...\n");
  /* these should not crash; they should return failure / sentinel */
  assert(cdsg_get_size(NULL) == 0);
  assert(cdsg_get_bytes(NULL) == NULL);
  assert(cdsg_app_b(NULL, 0x12) == 0);
  assert(cdsg_app_bs(NULL, (uint8_t[]){1, 2}, 2) == 0);
  assert(cdsg_app_dw(NULL, 0x1234u) == 0);
  assert(cdsg_reserve(NULL, 4) == SIZE_MAX);

  printf("  PASSED\n");
}

int main(void) {
  printf("\n=== Running Robust Code Segment Tests ===\n\n");
  test_create_free();
  test_append_byte_and_bs();
  test_append_dw_endianness();
  test_reserve_and_patch();
  test_reserve_zero_noop();
  // test_reserve_overflow_detection(); // THIS IS NOT RELEVANT FOR RESERVE in
  // PASS 1
  test_growth_and_capacity();
  test_null_argument_handling();
  printf("\n=== Tests complete ===\n");
  return 0;
}
