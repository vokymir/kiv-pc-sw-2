#ifndef GRAMMAR_IDENTIFIER_H
#define GRAMMAR_IDENTIFIER_H

#include "grammar.h"

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
                                       const struct Token *tokens[],
                                       size_t segment_idx);

enum Err_Grm grammar_identifier_db_dec(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dec2(struct Parsed_Statement *pstmt,
                                        const struct Token *tokens[]);

enum Err_Grm grammar_identifier_db_dup(struct Parsed_Statement *pstmt,
                                       const struct Token *tokens[],
                                       size_t segment_idx);

#endif
