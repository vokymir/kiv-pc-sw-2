#ifndef GRAMMAR_INSTRUCTION_H
#define GRAMMAR_INSTRUCTION_H

#include "parser_grammar.h"

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]);

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]);

// ===== OPERANDS HELPER DECLARATIONS =====

// set both operands
int _set_ops_none(struct Instruction_Statement *is);

// set is->op[idx] to label of token->value
int _set_op_label(struct Instruction_Statement *is, const struct Token *token,
                  size_t idx);

// set is->op[idx] to register of token->value
int _set_op_register(struct Instruction_Statement *is,
                     const struct Token *token, size_t idx);

// set is->op[idx] to number saved in string format in token->value
int _set_op_number(struct Instruction_Statement *is, const struct Token *token,
                   size_t idx);

// set is->op[idx] to offset, token must point to identifier
int _set_op_offset(struct Instruction_Statement *is, const struct Token *token,
                   size_t idx);

// set is->op_count to idx+1 if possible.
int _set_op_count(struct Instruction_Statement *is, size_t idx);

#endif
