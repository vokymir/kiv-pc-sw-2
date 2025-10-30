#ifndef OUTPUT_H
#define OUTPUT_H

#include "assembler.h"
#include "common.h"

// Output correct binary from asp to asp->config->target.
// Ensure correct KMA header, order of segments in file, etc.
enum Err_Main output_binary(const struct Assembler_Processing *asp);

#endif
