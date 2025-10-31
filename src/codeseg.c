#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "codeseg.h"
#include "common.h"
#include "memory.h"

// Grow the buffer if needed.
// Ensures the buffer have cdsg->size + additional_b.
// Return 0 on failure, 1 on success.
static int _cdsg_ensure_capacity(struct Code_Segment *cdsg,
                                 size_t additional_b);

struct Code_Segment *cdsg_create(void) {
  struct Code_Segment *cdsg = NULL;
  cdsg = jalloc(sizeof(struct Code_Segment));
  CLEANUP_IF_FAIL(cdsg);

  cdsg->size = 0;
  cdsg->capacity = CDSG_INITIAL_CAPACITY;

  cdsg->bytes = jalloc(cdsg->capacity);
  CLEANUP_IF_FAIL(cdsg->bytes);

  return cdsg;

cleanup:
  if (cdsg) {
    jree(cdsg);
  }
  return NULL;
}

void cdsg_free(struct Code_Segment **cdsg) {
  if (!cdsg || !*cdsg) {
    return;
  }
  if ((*cdsg)->bytes) {
    jree((*cdsg)->bytes);
  }
  jree(*cdsg);
  *cdsg = NULL;
  return;
}

int cdsg_app_b(struct Code_Segment *cdsg, uint8_t b) {
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes);

  if (!_cdsg_ensure_capacity(cdsg, 1)) {
    return 0;
  }

  cdsg->bytes[cdsg->size++] = b;
  return 1;

cleanup:
  return 0;
}

int cdsg_app_bs(struct Code_Segment *cdsg, const uint8_t *bs, size_t count) {
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes && bs);

  if (count == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_cdsg_ensure_capacity(cdsg, count));

  memcpy(cdsg->bytes + cdsg->size, bs, count);
  cdsg->size += count;
  return 1;

cleanup:
  return 0;
}

int cdsg_app_op(struct Code_Segment *cdsg, uint8_t opcode) {
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes);

  return cdsg_app_b(cdsg, opcode);

cleanup:
  return 0;
}

int cdsg_app_reg(struct Code_Segment *cdsg, uint8_t reg_code) {
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes);

  return cdsg_app_b(cdsg, reg_code);

cleanup:
  return 0;
}

int cdsg_app_imm(struct Code_Segment *cdsg, int32_t imm32b_v) {
  uint8_t bytes[4];
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes);

  bytes[0] = (uint8_t)((imm32b_v >> 0) & 0xFF);
  bytes[1] = (uint8_t)((imm32b_v >> 8) & 0xFF);
  bytes[2] = (uint8_t)((imm32b_v >> 16) & 0xFF);
  bytes[3] = (uint8_t)((imm32b_v >> 24) & 0xFF);

  return cdsg_app_bs(cdsg, bytes, 4);

cleanup:
  return 0;
}

size_t cdsg_get_size(const struct Code_Segment *cdsg) {
  CLEANUP_IF_FAIL(cdsg);

  return cdsg->size;

cleanup:
  return 0;
}

const uint8_t *cdsg_get_bytes(const struct Code_Segment *cdsg) {
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes);

  return cdsg->bytes;

cleanup:
  return NULL;
}

size_t cdsg_advance(struct Code_Segment *cdsg, size_t num_bytes) {
  size_t pos = 0;
  CLEANUP_IF_FAIL(cdsg);

  if (cdsg->size > SIZE_MAX - num_bytes) {
    goto cleanup;
  }

  pos = cdsg->size;
  cdsg->size += num_bytes;
  return pos;

cleanup:
  return SIZE_MAX;
}

static int _cdsg_ensure_capacity(struct Code_Segment *cdsg,
                                 size_t additional_b) {
  size_t req = 0, new_c = 0;
  uint8_t *new_b = NULL;
  CLEANUP_IF_FAIL(cdsg && cdsg->bytes);

  if (additional_b == 0) {
    return 1;
  }

  CLEANUP_IF_FAIL(cdsg->size <= SIZE_MAX - additional_b); // add overflow

  req = cdsg->size + additional_b;
  if (req <= cdsg->capacity) {
    return 1; // Already have enough space.
  }

  new_c = cdsg->capacity ? cdsg->capacity : CDSG_INITIAL_CAPACITY;
  while (new_c < req) {
    CLEANUP_IF_FAIL(new_c <=
                    SIZE_MAX / CDSG_CAPACITY_MULT); // multiply overflow
    new_c *= CDSG_CAPACITY_MULT;
  }

  new_b = jealloc(cdsg->bytes, new_c);
  CLEANUP_IF_FAIL(new_b); // realloc failed

  cdsg->bytes = new_b;
  cdsg->capacity = new_c;
  return 1;

cleanup:
  return 0;
}
