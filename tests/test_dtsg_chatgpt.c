// tests/test_dataseg.c
#include "../src/dataseg.h"
#include <assert.h>
#include <limits.h> // for SIZE_MAX
#include <stdint.h>
#include <stdio.h>
#include <string.h>

// Helper to read non-const pointer for checks (we only read)
static const uint8_t *bytes(const struct Data_Segment *d) {
  return dtsg_get_bytes(d);
}

static void test_create_free(void) {
  printf("Testing create/free...\n");
  struct Data_Segment *d = dtsg_create();
  assert(d != NULL);
  assert(d->size == 0);
  assert(d->capacity >= DTSG_INITIAL_CAPACITY);
  dtsg_free(d);
  printf("  PASSED\n");
}

static void test_append_byte_and_bytes(void) {
  printf("Testing append byte(s)...\n");
  struct Data_Segment *d = dtsg_create();
  assert(d != NULL);

  // append single bytes
  assert(dtsg_app_b(d, 0xAA) == 1);
  assert(dtsg_app_b(d, 0xBB) == 1);
  assert(dtsg_get_size(d) == 2);
  const uint8_t *p = bytes(d);
  assert(p[0] == 0xAA && p[1] == 0xBB);

  // append byte array
  uint8_t arr[] = {0x01, 0x02, 0x03, 0x04};
  assert(dtsg_app_bs(d, arr, sizeof(arr)) == 1);
  assert(dtsg_get_size(d) == 6);
  p = bytes(d);
  // previously 0xAA,0xBB then arr
  assert(p[2] == 0x01 && p[5] == 0x04);

  dtsg_free(d);
  printf("  PASSED\n");
}

static void test_append_dword_and_dws(void) {
  printf("Testing append dword(s) (little-endian)...\n");
  struct Data_Segment *d = dtsg_create();
  assert(d != NULL);

  // Append a single dword 0x11223344 -> bytes: 44 33 22 11
  assert(dtsg_app_dw(d, 0x11223344) == 1);
  assert(dtsg_get_size(d) == 4);
  const uint8_t *p = bytes(d);
  assert(p[0] == 0x44 && p[1] == 0x33 && p[2] == 0x22 && p[3] == 0x11);

  // Append an array of two dwords: 1 and -1
  int32_t arr[] = {1, -1};
  assert(dtsg_app_dws(d, arr, 2) == 1);
  // total size now 4 + 2*4 = 12
  assert(dtsg_get_size(d) == 12);
  p = bytes(d);
  // Check the first appended dword (already checked) and the appended ones:
  // offset 4: dword 1 -> 01 00 00 00
  assert(p[4] == 0x01 && p[5] == 0x00 && p[6] == 0x00 && p[7] == 0x00);
  // offset 8: dword -1 -> FF FF FF FF
  assert(p[8] == 0xFF && p[9] == 0xFF && p[10] == 0xFF && p[11] == 0xFF);

  dtsg_free(d);
  printf("  PASSED\n");
}

static void test_append_string_and_zeroes(void) {
  printf("Testing append string and zeroes...\n");
  struct Data_Segment *d = dtsg_create();
  assert(d != NULL);

  const char *s = "hello";
  // append string - should include terminating null according to header comment
  assert(dtsg_app_str(d, s) == 1);
  size_t sz = dtsg_get_size(d);
  const uint8_t *p = bytes(d);
  // check 'h','e','l','l','o','\0'
  assert(sz >= 6);
  assert(memcmp(p, "hello", 5) == 0);
  assert(p[5] == 0);

  // append 4 zero bytes
  assert(dtsg_app_zs(d, 4) == 1);
  size_t new_sz = dtsg_get_size(d);
  assert(new_sz == sz + 4);
  p = bytes(d);
  // last 4 bytes should be zero
  assert(p[new_sz - 1] == 0 && p[new_sz - 4] == 0);

  dtsg_free(d);
  printf("  PASSED\n");
}

static void test_capacity_growth_and_large_append(void) {
  printf("Testing capacity growth with many appends...\n");
  struct Data_Segment *d = dtsg_create();
  assert(d != NULL);

  size_t initial_cap = d->capacity;
  // append more than initial capacity to force growth
  size_t to_append = initial_cap * 4 + 3; // definitely larger
  uint8_t pattern = 0x7F;
  for (size_t i = 0; i < to_append; ++i) {
    assert(dtsg_app_b(d, pattern) == 1);
  }
  assert(dtsg_get_size(d) == to_append);
  assert(d->capacity >= to_append);
  // check a few samples
  const uint8_t *p = bytes(d);
  assert(p[0] == pattern);
  assert(p[to_append - 1] == pattern);

  dtsg_free(d);
  printf("  PASSED\n");
}

static void test_advance_behavior(void) {
  printf("Testing dtsg_advance behavior...\n");
  struct Data_Segment *d = dtsg_create();
  assert(d != NULL);

  // first advance by 10 -> should return offset 0 and size becomes 10
  size_t off1 = dtsg_advance(d, 10);
  assert(off1 != SIZE_MAX);
  assert(off1 == 0);
  assert(dtsg_get_size(d) >= 10);

  // second advance by 5 -> should return old size (10) and size becomes 15
  size_t off2 = dtsg_advance(d, 5);
  assert(off2 != SIZE_MAX);
  assert(off2 == off1 + 10);
  assert(dtsg_get_size(d) >= 15);

  // appending after advance appends after advanced region
  assert(dtsg_app_b(d, 0xAB) == 1);
  size_t final_size = dtsg_get_size(d);
  const uint8_t *p = bytes(d);
  assert(p[final_size - 1] == 0xAB);

  dtsg_free(d);
  printf("  PASSED\n");
}

int main(void) {
  printf("\n=== Running Data_Segment Unit Tests ===\n\n");
  test_create_free();
  test_append_byte_and_bytes();
  test_append_dword_and_dws();
  test_append_string_and_zeroes();
  test_capacity_growth_and_large_append();
  test_advance_behavior();
  printf("\n=== All Data_Segment Tests Passed ===\n\n");
  return 0;
}
