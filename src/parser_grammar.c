#include "parser_grammar.h"
#include "common.h"

int grammar_line(struct Parsed_Statement *pstmt, const struct Token *tokens[]) {
  CLEANUP_IF_FAIL(pstmt && tokens && *tokens);

  return grammar_line_kma(pstmt, tokens) || grammar_line_code(pstmt, tokens) ||
         grammar_line_data(pstmt, tokens) ||
         grammar_line_label(pstmt, tokens) ||
         grammar_line_identifier(pstmt, tokens) ||
         grammar_line_instruction(pstmt, tokens) || grammar_eof(pstmt, tokens);

cleanup:
  return 0;
}
