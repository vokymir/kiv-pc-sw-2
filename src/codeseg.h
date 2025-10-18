#ifndef CODESEG_H
#define CODESEG_H

#include <stddef.h>
#include <stdint.h>

struct Code_Segment {
  uint8_t *bytes;  // byte buffer
  size_t size;     // currently used
  size_t capacity; // are allocated
};

#endif
