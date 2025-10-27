#ifndef PARSER_GRAMMAR_H
#define PARSER_GRAMMAR_H

/* DESCRIPTION OF GRAMMAR
 * - terminating symbols are in UPPERCASE and have direct equivalent in enum
 * Token_Type
 * - non-terminating in <> and these are named same as their functions
 * - its ALMOST 3R grammar according to Chomsky:
 *  - on left side can only be one non-terminal
 *  - on right-hand-side can only be:
 *   - TERMINAL, EOF
 *   - TERMINAL, <non-terminal>
 *   ! EOF after TERMINAL could be viewed as violation of 3R grammar !
 *   ! <*dup> are clear violation !
 *   ! <instruction_*> are clear violation !
 * - every possible <line> must be ended by EOF
 *
 * 1) <line> --> <kma_line> | <code_line> | <data_line> | <label_line> |
 * <identifier_line> | <instruction_line> | EOF
 *
 * 2) <kma_line> --> KMA, EOF
 * 3) <code_line> --> CODE, EOF
 * 4) <data_line> --> DATA, EOF
 * 5) <label_line> --> LABEL, EOF
 *
 * 6) <identifier_line> --> IDENTIFIER, <identifier_def>
 * 7) <identifier_def> --> DATA_TYPE_DW, <identifier_dw_dec> | DATA_TYPE_DB,
 * <identifier_db_dec>
 * 8) <identifier_dw_dec> --> QUESTION, <identifier_dw_dec2> | NUMBER,
 * <identifier_dw_dec2> | <identifier_dw_dup>
 * 9) <identifier_dw_dec2> --> COMMA, <identifier_dw_dec> | EOF
 * 10) <identifier_dw_dup> --> NUMBER, DUP, LPAREN,
 * NUM, RPAREN, <identifier_dw_dec2> | NUMBER, DUP, LPAREN, QUESTION, RPAREN,
 * <identifier_dw_dec2>
 * 11) <identifier_db_dec> --> QUESTION, <identifier_db_dec2> | NUMBER,
 * <identifier_db_dec2> | STRING, <identifier_db_dec2> | <identifier_db_dup> 12)
 * <identifier_db_dec2> --> COMMA, <identifier_db_dec> | EOF 13)
 * <identifier_db_dup> --> NUMBER, DUP, LPAREN, NUM, RPAREN,
 * <identifier_db_dec2> | NUMBER, DUP, LPAREN, QUESTION, RPAREN,
 * <identifier_db_dec2>
 *
 * POZOR: MOHL BY BYT PROBLEM S DUP: PROTOZE TAKY ZACINA NA NUMBER
 *
 * 14) <instruction_line> --> INSTRUCTION, <instruction_rhs>
 * 15) <instruction_rhs> --> EOF | LABEL, EOF | REG, EOF | NUM, EOF | REG,
 * COMMA, <instruction_rhs_after>
 * 16) <instruction_rhs_after> --> REG, EOF | NUMBER, EOF | OFFSET, IDENTIFIER,
 * EOF
 */

#include "common.h"
#include "lexer.h"
#include "parser.h"

enum Err_Grm {
  GRM_MATCH,
  GRM_NO_MATCH,
  GRM_GENERIC_ERROR,
};

// VSECHNY FCE POKUD VOLAJI NEJAKOU DALSI, PREDAJI POINTER UZ POSUNUTEJ NA DALSI
// TOKEN

// Using grammar, parse the line. Update the pstmt to hold the answer.
// On success return 1 AND fill the pstmt, so the caller must free.
// On failure return 0 and allocates nothing.
enum Err_Grm grammar_line(struct Parsed_Statement *pstmt,
                          const struct Token *tokens[]);

// Evaluates whether the array tokens consists of TOKEN_KMA and TOKEN_EOF.
// On success sets the pstmt adequately and return GRM_MATCH.
// On failure return GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_line_kma(struct Parsed_Statement *pstmt,
                              const struct Token *tokens[]);

// Evaluates whether tokens consists of TOKEN_SECTION_CODE and TOKEN_EOF.
// On success set the pstmt adequately and return GRM_MATCH.
// On failure return GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_line_code(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]);

// Evaluates whether tokens consists of TOKEN_SECTION_DATA and TOKEN_EOF.
// On success set the pstmt adequately and return GRM_MATCH.
// On failure return GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_line_data(struct Parsed_Statement *pstmt,
                               const struct Token *tokens[]);

// Evaluates whether tokens consists of TOKEN_LABEL and TOKEN_EOF.
// On success return GRM_MATCH and set the pstmt - copying the label name to
// pstmt. On failure return GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_line_label(struct Parsed_Statement *pstmt,
                                const struct Token *tokens[]);

// Evaluates whether tokens array is a data definition statement.
// On success return GRM_MATCH and set the pstmt, copy identifier name to pstmt
// and call other functions to fill the insides of pstmt. On failure return
// GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_line_identifier(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]);

enum Err_Grm grammar_line_instruction(struct Parsed_Statement *pstmt,
                                      const struct Token *tokens[]);

// Evaluates whether the next token(s) is valid data type and when is, calls
// other functions to get what the insides are. On success set the pstmt
// declaration type, call other functions to fill the insides and return
// GRM_MATCH. On failure return GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_identifier_def(struct Parsed_Statement *pstmt,
                                    const struct Token *tokens[]);

// Evaluates whether token(s) is a valid data declaration, based on syntax.
// On success call other functions to search the rest of tokens, fill one
// segment in Data_Declaration and return GRM_MATCH. On failure return
// GRM_NO_MATCH and the pstmt is unchanged.
enum Err_Grm grammar_identifier_dw_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

// Evaluates whether token is comma or eof. If eof, it's success and setting
// pstmt takes place - based on the pstmt data_decl segment count it is
// allocated place for segments. If comma, other function for getting the next
// part is called. On any success return GRM_MATCH. On failure, pstmt is cleared
// and GRM_NO_MATCH is returned.
enum Err_Grm grammar_identifier_dw_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]);

// Checks if next tokens are valid DUP statement, remember what its parameters
// are and call other functions to look ahead in the data declaration. On
// success fills its segment in data_decl with all info and return GRM_MATCH. On
// failure return GRM_NO_MATCH.
enum Err_Grm grammar_identifier_dw_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_instruction_rhs(struct Parsed_Statement *pstmt,
                                     const struct Token *tokens[]);

enum Err_Grm grammar_instruction_rhs_after(struct Parsed_Statement *pstmt,
                                           const struct Token *tokens[]);

// Return GRM_MATCH if the token is EOF.
enum Err_Grm grammar_eof(const struct Parsed_Statement *pstmt,
                         const struct Token *tokens[]);
#endif
