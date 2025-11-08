#ifndef ASSEMBLER_PASSES_H
#define ASSEMBLER_PASSES_H
/* Module helper for functionality shared between both passes. */

#include "assembler.h"

// Pass 1 & 2 holds many similarities, therefore this function exist to have the
// shared logic. Reads all lines from source .kas file and process them by
// calling different function for pass1 and 2.
enum Err_Asm passes_any_pass(struct Assembler_Processing *asp, int is_second);

// Pass 1 & 2 have very similiar behaviour on one line.
// This function have the common logic. Tokenize & parse the line, but then
// decide based on if <is_second>.
enum Err_Asm passes_line(struct Assembler_Processing *asp,
                         enum Assembler_Context *ctx, size_t nl,
                         const char *line, int is_second);

// Set context for ".KMA" line. Return error codes for no error or duplicate
// KMA lines.
enum Err_Asm passes_kma(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl);

// Set context for ".CODE" line. Return error code for no error or error if this
// line preceeds ".KMA" line.
enum Err_Asm passes_code_section(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

// Set context for ".DATA" line. Return error code for no error or error if this
// line preceeds ".KMA" line.
enum Err_Asm passes_data_section(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

// Process empty (or only comment) line.
enum Err_Asm passes_none(struct Assembler_Processing *asp, size_t nl);

// Process line containing error.
enum Err_Asm passes_error(struct Assembler_Processing *asp, size_t nl);

#endif
