// tests/test_codeseg.c
#include "../src/codeseg.h"
#include "../src/memory.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void print_bytes(const uint8_t *b, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    printf("%02X ", b[i]);
  }
  printf("\n");
}

static void test_create_free(void) {
  printf("test_create_free...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);
  assert(cdsg_get_size(cd) == 0);
  const uint8_t *bytes = cdsg_get_bytes(cd);
  assert(bytes != NULL || cdsg_get_size(cd) == 0); // bytes pointer available
  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

static void test_append_byte_and_bytes(void) {
  printf("test_append_byte_and_bytes...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  // append single bytes
  assert(cdsg_app_b(cd, 0xAA) == 1);
  assert(cdsg_app_b(cd, 0xBB) == 1);
  assert(cdsg_get_size(cd) == 2);

  // append buffer
  uint8_t buf[] = {0x01, 0x02, 0x03, 0x04};
  assert(cdsg_app_bs(cd, buf, sizeof(buf)) == 1);
  assert(cdsg_get_size(cd) == 2 + sizeof(buf));

  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[0] == 0xAA && b[1] == 0xBB);
  for (size_t i = 0; i < sizeof(buf); ++i) {
    assert(b[2 + i] == buf[i]);
  }

  printf("  bytes: ");
  print_bytes(b, cdsg_get_size(cd));

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

static void test_append_dw_endianness(void) {
  printf("test_append_dw_endianness...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  // Append dword 0x11223344; expect bytes 44 33 22 11
  int32_t dw = 0x11223344;
  assert(cdsg_app_dw(cd, dw) == 1);
  assert(cdsg_get_size(cd) == 4);

  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[0] == 0x44 && b[1] == 0x33 && b[2] == 0x22 && b[3] == 0x11);

  printf("  dword bytes: ");
  print_bytes(b, cdsg_get_size(cd));

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

static void test_reserve_and_patch(void) {
  printf("test_reserve_and_patch...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  // reserve 4 bytes and later write there
  size_t pos = cdsg_reserve(cd, 4);
  assert(pos != (size_t)-1); // using SIZE_MAX as failure sentinel (see review)
  assert(cdsg_get_size(cd) >= pos + 4);

  // write little-endian dword at reserved location directly
  uint8_t *mutable = (uint8_t *)cdsg_get_bytes(cd);
  mutable[pos + 0] = 0xDE;
  mutable[pos + 1] = 0xAD;
  mutable[pos + 2] = 0xBE;
  mutable[pos + 3] = 0xEF;

  const uint8_t *b = cdsg_get_bytes(cd);
  assert(b[pos + 0] == 0xDE && b[pos + 3] == 0xEF);

  printf("  patched bytes at pos %zu: ", pos);
  print_bytes(&b[pos], 4);

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

static void test_growth_behavior(void) {
  printf("test_growth_behavior...\n");
  struct Code_Segment *cd = cdsg_create();
  assert(cd != NULL);

  // Append more bytes than initial capacity to force growth
  size_t to_append = CDSG_INITIAL_CAPACITY + 10;
  for (size_t i = 0; i < to_append; ++i) {
    assert(cdsg_app_b(cd, (uint8_t)(i & 0xFF)) == 1);
  }
  assert(cdsg_get_size(cd) == to_append);

  const uint8_t *b = cdsg_get_bytes(cd);
  for (size_t i = 0; i < to_append; ++i) {
    assert(b[i] == (uint8_t)(i & 0xFF));
  }
  printf("  appended %zu bytes, final size %zu\n", to_append,
         cdsg_get_size(cd));

  cdsg_free(cd);
  assert(jemory() == 0);
  printf("  PASSED\n");
}

int main(void) {
  printf("\n=== Running Code Segment Tests ===\n\n");
  test_create_free();
  test_append_byte_and_bytes();
  test_append_dw_endianness();
  test_reserve_and_patch();
  test_growth_behavior();
  printf("\n=== All Code Segment Tests Passed! ===\n\n");
  return 0;
}
