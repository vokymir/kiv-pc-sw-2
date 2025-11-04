#ifndef GRAMMAR_INSTRUCTION_H
#define GRAMMAR_INSTRUCTION_H

#include "grammar.h"

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]);

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]);

#endif
