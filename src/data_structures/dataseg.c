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
  CLEANUP_IF_FAIL_ERR(dtsg, "Couldn't allocate data segment.");

  dtsg->size = 0;
  dtsg->capacity = DTSG_INITIAL_CAPACITY;

  dtsg->bytes = jalloc(dtsg->capacity);
  CLEANUP_IF_FAIL_ERR(dtsg->bytes, "Couldn't allocate data segment buffer.");

  return dtsg;

cleanup:
  if (dtsg) {
    jree(dtsg);
  }
  return NULL;
}

void dtsg_free(struct Data_Segment **dtsg) {
  if (!dtsg || !*dtsg) {
    PRINT_ERR("Tried to free already freed (or invalid pointer) data segment.");
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
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  if (!_dtsg_ensure_capacity(dtsg, KMA_BYTE_SIZE)) {
    return 0;
  }

  dtsg->bytes[dtsg->size++] = b;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_dw_n(struct Data_Segment *dtsg, int32_t dw, size_t n) {
  size_t i = 0;
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  if (n == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, n * KMA_DWORD_SIZE));

  for (i = 0; i < n; i++) {
    if (!dtsg_app_dw(dtsg, dw)) {
      PRINT_ERR("Couldn't append DWORD to data segment. It's %zu. dword when "
                "appending the same DWORD (%i) n (%zu) times.",
                i + 1, dw, n);
      return 0;
    }
  }

  return 1;

cleanup:
  return 0;
}

int dtsg_app_bs(struct Data_Segment *dtsg, const uint8_t *bs, size_t count) {
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes && bs,
                      "The given data segment or bytes array weren't valid.");

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

int dtsg_app_b_n(struct Data_Segment *dtsg, uint8_t b, size_t n) {
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  if (n == 0) {
    return 1; // nothing to do
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, n));

  memset(dtsg->bytes + dtsg->size, b, n);
  dtsg->size += n;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_dw(struct Data_Segment *dtsg, int32_t dw) {
  uint8_t bytes[KMA_DWORD_SIZE];
  size_t i = 0;
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  for (i = 0; i < KMA_DWORD_SIZE; i++) {
    bytes[i] = (uint8_t)((dw >> (i * 8)) & 0xFF);
  }

  return dtsg_app_bs(dtsg, bytes, KMA_DWORD_SIZE);

cleanup:
  return 0;
}

int dtsg_app_dws(struct Data_Segment *dtsg, const int32_t *dws, size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes && dws,
                      "The given data segment or DWORD array weren't valid.");

  for (i = 0; i < count; i++) {
    if (!dtsg_app_dw(dtsg, dws[i])) {
      PRINT_ERR("Couldn't append %zu. DWORD (32 bits: %i) to data segment.",
                i + 1, dws[i]);
      return 0;
    }
  }

  return 1;

cleanup:
  return 0;
}

int dtsg_app_str(struct Data_Segment *dtsg, const char *string) {
  size_t len = 0;
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes && string,
                      "The given data segment wasn't valid.");

  len = strlen(string);
  if (len == 0) {
    return 1; // nothing to append
  }

  CLEANUP_IF_FAIL(_dtsg_ensure_capacity(dtsg, len));

  memmove(dtsg->bytes + dtsg->size, string, len);
  dtsg->size += len;
  return 1;

cleanup:
  return 0;
}

int dtsg_app_zs(struct Data_Segment *dtsg, size_t count) {
  size_t i = 0;
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  for (i = 0; i < count; i++) {
    if (!dtsg_app_b(dtsg, 0)) {
      PRINT_ERR("Couldn't append uninitialized (0) byte to data segment. It's "
                "%zu. byte, when appending zero (%zu) times.",
                i + 1, count);
      return 0;
    }
  }

  return 1;

cleanup:
  return 0;
}

size_t dtsg_get_size(const struct Data_Segment *dtsg) {
  CLEANUP_IF_FAIL_ERR(dtsg, "The given data segment wasn't valid.");

  return dtsg->size;

cleanup:
  return 0;
}

const uint8_t *dtsg_get_bytes(const struct Data_Segment *dtsg) {
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  return dtsg->bytes;

cleanup:
  return NULL;
}

size_t dtsg_advance(struct Data_Segment *dtsg, size_t num_bytes) {
  size_t pos = 0;
  CLEANUP_IF_FAIL_ERR(dtsg, "The given data segment wasn't valid.");

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
    PRINT_ERR("The given data segment wasn't valid.");
    return 0;
  }

  dtsg->size = 0;
  return 1;
}

static int _dtsg_ensure_capacity(struct Data_Segment *dtsg,
                                 size_t additional_b) {
  size_t req = 0, new_c = 0;
  uint8_t *new_b = NULL;
  CLEANUP_IF_FAIL_ERR(dtsg && dtsg->bytes,
                      "The given data segment wasn't valid.");

  if (additional_b == 0) {
    return 1;
  }

  CLEANUP_IF_FAIL_ERR(
      dtsg->size <= SIZE_MAX - additional_b, // [before](see below)
      "When trying to calculate the new size of data segment "
      "buffer, the limit was reached and it caused an overflow.");

  req = dtsg->size + additional_b;
  if (req <= dtsg->capacity) {
    return 1; // Already have enough space.
  }

  new_c = dtsg->capacity ? dtsg->capacity : DTSG_INITIAL_CAPACITY;
  while (new_c < req) {
    // iI cannot multiply, because that would cause overflow, set to max value.
    // new capacity has to be at most equal SIZE_MAX, as we found before.
    if (new_c > SIZE_MAX / DTSG_CAPACITY_MULT) {
      new_c = SIZE_MAX;
      break;
    }
    new_c *= DTSG_CAPACITY_MULT;
  }

  new_b = jealloc(dtsg->bytes, new_c);
  CLEANUP_IF_FAIL_ERR(new_b,
                      "Couldn't grow the data segment buffer using realloc.");

  dtsg->bytes = new_b;
  dtsg->capacity = new_c;
  return 1;

cleanup:
  return 0;
}
