#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "dataseg.h"
#include "memory.h"

// Grow the buffer if needed.
// Ensures the buffer have dtsg->size + additional_b.
// Return 0 on failure, 1 on success.
static int _dtsg_ensure_capacity(struct Data_Segment *dtsg,
                                 size_t additional_b);

struct Data_Segment *dtsg_create(void) {
  struct Data_Segment *dtsg = NULL;
  dtsg = jalloc(sizeof(struct Data_Segment));
  CLEANUP_IF_FAIL(dtsg);

  dtsg->size = 0;
  dtsg->capacity = DTSG_INITIAL_CAPACITY;

  dtsg->bytes = jalloc(dtsg->capacity);
  CLEANUP_IF_FAIL(dtsg->bytes);

  return dtsg;

cleanup:
  if (dtsg) {
    jree(dtsg);
  }
  return NULL;
}

void dtsg_free(struct Data_Segment **dtsg) {
  if (!dtsg || !*dtsg) {
    return;
  }
  if ((*dtsg)->bytes) {
    jree((*dtsg)->bytes);
  }
  jree(*dtsg);
  *dtsg = NULL;
  return;
}

int dtsg_app_b(struct Data_Segment *dtsg, uint8_t b) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes);

  if (!_dtsg_ensure_capacity(dtsg, 1)) {
    return 0;
  }

  dtsg->bytes[dtsg->size++] = b;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_bs(struct Data_Segment *dtsg, const uint8_t *bs, size_t count) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && bs);

  if (count == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, count));

  memcpy(dtsg->bytes + dtsg->size, bs, count);
  dtsg->size += count;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_dw(struct Data_Segment *dtsg, int32_t dw) {
  uint8_t bytes[4];
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes);

  bytes[0] = (uint8_t)((dw >> 0) & 0xFF);
  bytes[1] = (uint8_t)((dw >> 8) & 0xFF);
  bytes[2] = (uint8_t)((dw >> 16) & 0xFF);
  bytes[3] = (uint8_t)((dw >> 24) & 0xFF);

  return dtsg_app_bs(dtsg, bytes, 4);

cleanup:
  return 0;
}

int dtsg_app_dws(struct Data_Segment *dtsg, const int32_t *dws, size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && dws);

  for (i = 0; i < count; i++) {
    if (!dtsg_app_dw(dtsg, dws[i])) {
      return 0;
    }
  }

  return 1;

cleanup:
  return 0;
}

int dtsg_app_str(struct Data_Segment *dtsg, const char *string) {
  size_t len = 0;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && string);

  len = strlen(string) + 1;
  return dtsg_app_bs(dtsg, (const uint8_t *)string, len);

cleanup:
  return 0;
}

int dtsg_app_zs(struct Data_Segment *dtsg, size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes);

  for (i = 0; i < count; i++) {
    if (!dtsg_app_b(dtsg, 0)) {
      return 0;
    }
  }

  return 1;

cleanup:
  return 0;
}

size_t dtsg_get_size(const struct Data_Segment *dtsg) {
  CLEANUP_IF_FAIL(dtsg);

  return dtsg->size;

cleanup:
  return 0;
}

const uint8_t *dtsg_get_bytes(const struct Data_Segment *dtsg) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes);

  return dtsg->bytes;

cleanup:
  return NULL;
}

size_t dtsg_advance(struct Data_Segment *dtsg, size_t num_bytes) {
  size_t pos = 0;
  CLEANUP_IF_FAIL(dtsg);

  if (dtsg->size > SIZE_MAX - num_bytes) {
    goto cleanup;
  }

  pos = dtsg->size;
  dtsg->size += num_bytes;
  return pos;

cleanup:
  return SIZE_MAX;
}

static int _dtsg_ensure_capacity(struct Data_Segment *dtsg,
                                 size_t additional_b) {
  size_t req = 0, new_c = 0;
  uint8_t *new_b = NULL;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes);

  if (additional_b == 0) {
    return 1;
  }

  CLEANUP_IF_FAIL(dtsg->size <= SIZE_MAX - additional_b); // add overflow

  req = dtsg->size + additional_b;
  if (req <= dtsg->capacity) {
    return 1; // Already have enough space.
  }

  new_c = dtsg->capacity ? dtsg->capacity : DTSG_INITIAL_CAPACITY;
  while (new_c < req) {
    CLEANUP_IF_FAIL(new_c <=
                    SIZE_MAX / DTSG_CAPACITY_MULT); // multiply overflow
    new_c *= DTSG_CAPACITY_MULT;
  }

  new_b = jealloc(dtsg->bytes, new_c);
  CLEANUP_IF_FAIL(new_b); // realloc failed

  dtsg->bytes = new_b;
  dtsg->capacity = new_c;
  return 1;

cleanup:
  return 0;
}
