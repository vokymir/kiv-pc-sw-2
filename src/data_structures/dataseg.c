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
  CLEANUP_IF_FAIL(dtsg, "TODO:");

  dtsg->size = 0;
  dtsg->capacity = DTSG_INITIAL_CAPACITY;

  dtsg->bytes = jalloc(dtsg->capacity);
  CLEANUP_IF_FAIL(dtsg->bytes, "TODO:");

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
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes, "TODO:");

  if (!_dtsg_ensure_capacity(dtsg, KMA_BYTE_SIZE)) {
    return 0;
  }

  dtsg->bytes[dtsg->size++] = b;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_dw_n(struct Data_Segment *dtsg, int32_t dw, size_t n) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes, "TODO:");

  if (n == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, n * KMA_DWORD_SIZE), "TODO:");

  for (size_t i = 0; i < n; i++) {
    if (!dtsg_app_dw(dtsg, dw)) {
      return 0;
    }
  }

  return 1;

cleanup:
  return 0;
}

int dtsg_app_bs(struct Data_Segment *dtsg, const uint8_t *bs, size_t count) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && bs, "TODO:");

  if (count == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, count), "TODO:");

  memcpy(dtsg->bytes + dtsg->size, bs, count);
  dtsg->size += count;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_b_n(struct Data_Segment *dtsg, uint8_t b, size_t n) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && b, "TODO:");

  if (n == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, n), "TODO:");

  memset(dtsg->bytes + dtsg->size, b, n);
  dtsg->size += n;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_dw(struct Data_Segment *dtsg, int32_t dw) {
  uint8_t bytes[KMA_DWORD_SIZE];
  size_t i = 0;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes, "TODO:");

  for (i = 0; i < KMA_DWORD_SIZE; i++) {
    bytes[i] = (uint8_t)((dw >> (i * 8)) &
                         0xFF); // TODO: hejdula tvrdi, ze 0xFF je hloupost
  }

  return dtsg_app_bs(dtsg, bytes, KMA_DWORD_SIZE);

cleanup:
  return 0;
}

int dtsg_app_dws(struct Data_Segment *dtsg, const int32_t *dws, size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && dws, "TODO:");

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
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes && string, "TODO:");

  len = strlen(string);
  if (len == 0) {
    return 1; // nothing to append
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, len), "TODO:");

  memmove(dtsg->bytes + dtsg->size, string, len);
  dtsg->size += len;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_zs(struct Data_Segment *dtsg, size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes, "TODO:");

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
  CLEANUP_IF_FAIL(dtsg, "TODO:");

  return dtsg->size;

cleanup:
  return 0;
}

const uint8_t *dtsg_get_bytes(const struct Data_Segment *dtsg) {
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes, "TODO:");

  return dtsg->bytes;

cleanup:
  return NULL;
}

size_t dtsg_advance(struct Data_Segment *dtsg, size_t num_bytes) {
  size_t pos = 0;
  CLEANUP_IF_FAIL(dtsg, "TODO:");

  if (dtsg->size > SIZE_MAX - num_bytes) {
    goto cleanup;
  }

  pos = dtsg->size;
  dtsg->size += num_bytes;
  return pos;

cleanup:
  return SIZE_MAX;
}

int dtsg_begin(struct Data_Segment *dtsg) {
  if (!dtsg) {
    return 0;
  }

  dtsg->size = 0;
  return 1;
}

static int _dtsg_ensure_capacity(struct Data_Segment *dtsg,
                                 size_t additional_b) {
  size_t req = 0, new_c = 0;
  uint8_t *new_b = NULL;
  CLEANUP_IF_FAIL(dtsg && dtsg->bytes, "TODO:");

  if (additional_b == 0) {
    return 1;
  }

  CLEANUP_IF_FAIL(dtsg->size <= SIZE_MAX - additional_b,
                  "TODO:"); // add overflow

  req = dtsg->size + additional_b;
  if (req <= dtsg->capacity) {
    return 1; // Already have enough space.
  }

  new_c = dtsg->capacity ? dtsg->capacity : DTSG_INITIAL_CAPACITY;
  while (new_c < req) {
    CLEANUP_IF_FAIL(new_c <= SIZE_MAX / DTSG_CAPACITY_MULT,
                    "TODO:"); // multiply overflow
    new_c *= DTSG_CAPACITY_MULT;
  }

  new_b = jealloc(dtsg->bytes, new_c);
  CLEANUP_IF_FAIL(new_b, "TODO:"); // realloc failed

  dtsg->bytes = new_b;
  dtsg->capacity = new_c;
  return 1;

cleanup:
  return 0;
}
