#ifndef ASSEMBLER_PASS_1_H
#define ASSEMBLER_PASS_1_H

#include "assembler.h"
#include <parser.h>
#include <stdlib.h>

// Process one line in the first pass of the assembler code.
// Return adequate error code, edit context if changed.
enum Err_Asm pass1_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line);

// Decide what to do in pass1 with given pstmt in pass one.
enum Err_Asm pass1_decide(struct Parsed_Statement *pstmt,
                          struct Assembler_Processing *asp,
                          enum Assembler_Context *ctx, size_t nl);

enum Err_Asm pass1_data_decl(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl);

enum Err_Asm pass1_instruction(struct Parsed_Statement *pstmt,
                               struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl);

enum Err_Asm pass1_label_def(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl);

#endif
