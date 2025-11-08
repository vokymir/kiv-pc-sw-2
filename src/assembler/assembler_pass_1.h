#ifndef ASSEMBLER_PASS_1_H
#define ASSEMBLER_PASS_1_H
/* Module helper for functionality specific to 1st pass. */

#include <parser.h>
#include <stdlib.h>

#include "assembler.h"

// Process one <line> in the first pass of the assembler code.
// Might edit <context> or any structure inside <asp>, based on what is on the
// given <line>. Return adequate error code.
enum Err_Asm pass1_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line);

// Decide which function is called based on information from already parsed line
// saved inside <pstmt>. Pass the error the called function returned.
enum Err_Asm pass1_decide(struct Parsed_Statement *pstmt,
                          struct Assembler_Processing *asp,
                          enum Assembler_Context *ctx, size_t nl);

// Process data declaration in .DATA section in the 1st pass. "Reserve" the
// space for data by advancing the dtsg inside <asp> and store the name to
// symbol table inside <asp>.
enum Err_Asm pass1_data_decl(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl);

// Process instruction in .CODE section in 1st pass by "reserving" the space for
// it in the cdsg inside <asp>.
enum Err_Asm pass1_instruction(struct Parsed_Statement *pstmt,
                               struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl);

// Process label definition in .CODE section in 1st pass by saving it to the
// symbol table inside <asp>.
enum Err_Asm pass1_label_def(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl);

#endif
