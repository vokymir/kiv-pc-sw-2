#ifndef ASSEMBLER_PASSES_H
#define ASSEMBLER_PASSES_H

#include "assembler.h"

// Pass 1 & 2 holds many similarities, therefore this function to have the
// shared logic. Calls different functions based on the pass.
enum Err_Asm passes_any_pass(struct Assembler_Processing *asp, int is_second);

// Pass 1 & 2 have very similiar behaviour on one line.
// This function have the common logic & calls adequate functions based on
// is_second.
enum Err_Asm passes_line(struct Assembler_Processing *asp,
                         enum Assembler_Context *ctx, size_t nl,
                         const char *line, int is_second);

// Perform all actions required for first pass, case STMT_KMA.
// Return adequate err_asm, preferably asm_no_error
enum Err_Asm passes_kma(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl);

enum Err_Asm passes_code_section(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

enum Err_Asm passes_data_section(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

enum Err_Asm passes_none(struct Assembler_Processing *asp, size_t nl);

enum Err_Asm passes_error(struct Assembler_Processing *asp, size_t nl);

#endif
