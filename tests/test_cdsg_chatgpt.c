#include "../src/codeseg.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- UTILITIES ---

static void print_header(const char *name) { printf("\n=== %s ===\n", name); }

// Helper: fill memory with predictable pattern
static void fill_pattern(uint8_t *buf, size_t n, uint8_t start) {
  for (size_t i = 0; i < n; ++i)
    buf[i] = start + (uint8_t)i;
}

// --- TESTS ---

static void test_create_and_free(void) {
  print_header("test_create_and_free");

  struct Code_Segment *seg = cdsg_create();
  assert(seg != NULL);
  assert(seg->bytes != NULL);
  assert(seg->capacity >= CDSG_INITIAL_CAPACITY);
  assert(seg->size == 0);

  cdsg_free(&seg);
  printf("✅ create_and_free passed.\n");
}

static void test_append_single_bytes(void) {
  print_header("test_append_single_bytes");

  struct Code_Segment *seg = cdsg_create();
  assert(seg);

  for (int i = 0; i < 32; ++i) {
    int ok = cdsg_app_b(seg, (uint8_t)i);
    assert(ok);
  }

  assert(cdsg_get_size(seg) == 32);
  const uint8_t *bytes = cdsg_get_bytes(seg);
  for (int i = 0; i < 32; ++i) {
    assert(bytes[i] == (uint8_t)i);
  }

  cdsg_free(&seg);
  printf("✅ append_single_bytes passed.\n");
}

static void test_append_byte_array(void) {
  print_header("test_append_byte_array");

  struct Code_Segment *seg = cdsg_create();
  assert(seg);

  uint8_t data[8];
  fill_pattern(data, sizeof(data), 0x10);

  int ok = cdsg_app_bs(seg, data, sizeof(data));
  assert(ok);
  assert(cdsg_get_size(seg) == sizeof(data));

  const uint8_t *b = cdsg_get_bytes(seg);
  for (size_t i = 0; i < sizeof(data); ++i) {
    assert(b[i] == (uint8_t)(0x10 + i));
  }

  cdsg_free(&seg);
  printf("✅ append_byte_array passed.\n");
}

static void test_append_opcode_reg_imm(void) {
  print_header("test_append_opcode_reg_imm");

  struct Code_Segment *seg = cdsg_create();
  assert(seg);

  assert(cdsg_app_op(seg, 0xAB));
  assert(cdsg_app_reg(seg, 0xCD));
  assert(cdsg_app_imm(seg, 0x12345678));

  assert(cdsg_get_size(seg) == 1 + 1 + 4);

  const uint8_t *b = cdsg_get_bytes(seg);
  assert(b[0] == 0xAB);
  assert(b[1] == 0xCD);

  // Immediate should be little-endian (implementation-dependent, but usually
  // is)
  int32_t imm;
  memcpy(&imm, b + 2, 4);
  assert(imm == 0x12345678);

  cdsg_free(&seg);
  printf("✅ append_opcode_reg_imm passed.\n");
}

static void test_capacity_growth(void) {
  print_header("test_capacity_growth");

  struct Code_Segment *seg = cdsg_create();
  assert(seg);

  size_t initial_cap = seg->capacity;

  // Append more bytes than initial capacity
  for (int i = 0; i < (int)(initial_cap * 3); ++i) {
    assert(cdsg_app_b(seg, (uint8_t)i));
  }

  assert(seg->capacity >= initial_cap * CDSG_CAPACITY_MULT);
  assert(seg->size == initial_cap * 3);

  cdsg_free(&seg);
  printf("✅ capacity_growth passed.\n");
}

static void test_advance_and_write(void) {
  print_header("test_advance_and_write");

  struct Code_Segment *seg = cdsg_create();
  assert(seg);

  size_t offset = cdsg_advance(seg, 10);
  assert(offset == 0);
  assert(cdsg_get_size(seg) == 10);

  offset = cdsg_advance(seg, 5);
  assert(offset == 10);
  assert(cdsg_get_size(seg) == 15);

  // Verify memory is still writable via cdsg_app_b
  assert(cdsg_app_b(seg, 0xFF));
  assert(cdsg_get_size(seg) == 16);

  cdsg_free(&seg);
  printf("✅ advance_and_write passed.\n");
}

// --- MAIN ---

int main(void) {
  printf("Running Code Segment module tests...\n");

  test_create_and_free();
  test_append_single_bytes();
  test_append_byte_array();
  test_append_opcode_reg_imm();
  test_capacity_growth();
  test_advance_and_write();

  printf("\nAll Code Segment tests passed successfully.\n");
  return 0;
}
