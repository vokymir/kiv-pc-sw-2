#ifndef ASSEMBLER_PASSES_H
#define ASSEMBLER_PASSES_H

#include "assembler.h"

// Pass 1 & 2 holds many similarities, therefore this function to have the
// shared logic. Calls different functions based on the pass.
enum Err_Asm _pass(struct Assembler_Processing *asp, int is_second);

// Pass 1 & 2 have very similiar behaviour on one line.
// This function have the common logic & calls adequate functions based on
// is_second.
enum Err_Asm _pass_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line, int is_second);

#endif
