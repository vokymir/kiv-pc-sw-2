#ifndef OUTPUT_H
#define OUTPUT_H
/* Module for writing the prepared binary to the target file. Uses structures
 * storing data/code in <asp> and make sure thr outputted binary is valid KMX
 * file. */

#include "assembler.h"
#include "common.h"

// Output correct binary from <asp> to target file.
// Ensure KMA header, order of segments in binary file, etc.
// Return standard error.
enum Err_Main output_binary(const struct Assembler_Processing *asp);

#endif
