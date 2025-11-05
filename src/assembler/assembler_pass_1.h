#ifndef ASSEMBLER_PASS_1_H
#define ASSEMBLER_PASS_1_H

#include "assembler.h"
#include <parser.h>
#include <stdlib.h>

// Process one line in the first pass of the assembler code.
// Return adequate error code, edit context if changed.
enum Err_Asm _pass1_line(struct Assembler_Processing *asp,
                         enum Assembler_Context *ctx, size_t nl,
                         const char *line);

// Decide what to do in pass1 with given pstmt in pass one.
enum Err_Asm _pass1_decide(struct Parsed_Statement *pstmt,
                           struct Assembler_Processing *asp,
                           enum Assembler_Context *ctx, size_t nl);

// Perform all actions required for first pass, case STMT_KMA.
// Return adequate err_asm, preferably asm_no_error
enum Err_Asm _pass1_kma(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl);

enum Err_Asm _pass1_code_section(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

enum Err_Asm _pass1_data_section(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

enum Err_Asm _pass1_data_decl(struct Parsed_Statement *pstmt,
                              struct Assembler_Processing *asp,
                              enum Assembler_Context *ctx, size_t nl);

enum Err_Asm _pass1_instruction(struct Parsed_Statement *pstmt,
                                struct Assembler_Processing *asp,
                                enum Assembler_Context *ctx, size_t nl);

enum Err_Asm _pass1_label_def(struct Parsed_Statement *pstmt,
                              struct Assembler_Processing *asp,
                              enum Assembler_Context *ctx, size_t nl);

enum Err_Asm _pass1_none(struct Assembler_Processing *asp, size_t nl);

enum Err_Asm _pass1_error(struct Assembler_Processing *asp, size_t nl);

#endif
