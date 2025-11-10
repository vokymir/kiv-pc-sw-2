#ifndef GRAMMAR_INSTRUCTION_H
#define GRAMMAR_INSTRUCTION_H

#include "parser_grammar.h"

// Check if tokens array is a right hand side of an instruction, the left side
// being the name of the instruction. If found match, set pstmt and return. On
// no match return NO_MATCH.
enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]);

// If found instruction with more than one argument, this function is called.
// Return GRM_(NO_)MATCH on success(failure) and pstmt is(not) modified.
enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]);

// ===== OPERANDS HELPER DECLARATIONS =====

// set both operands to OP_NONE
// Return 1 on success 0 on failure.
int _set_ops_none(struct Instruction_Statement *is);

// set is->op[idx] to label of token->value
// Return 1 on success 0 on failure.
int _set_op_label(struct Instruction_Statement *is, const struct Token *token,
                  size_t idx);

// set is->op[idx] to register of token->value
// Return 1 on success 0 on failure.
int _set_op_register(struct Instruction_Statement *is,
                     const struct Token *token, size_t idx);

// set is->op[idx] to parsed number (saved in string format in token->value)
// Return 1 on success 0 on failure.
int _set_op_number(struct Instruction_Statement *is, const struct Token *token,
                   size_t idx);

// set is->op[idx] to offset, token must point to identifier
// Return 1 on success 0 on failure.
int _set_op_offset(struct Instruction_Statement *is, const struct Token *token,
                   size_t idx);

// set is->op_count to idx+1 if possible.
// if is->op_count > idx+1, no overwrite.
// Return 1 on success 0 on failure.
int _set_op_count(struct Instruction_Statement *is, size_t idx);

#endif
