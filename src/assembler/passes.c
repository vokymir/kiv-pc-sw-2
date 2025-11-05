#include "assembler_convert.h"
#include "assembler_pass_1.h"
#include "assembler_pass_2.h"
#include "assembler_passes.h"
#include "fileutil.h"
#include "internal.h"
#include "memory.h"
#include <stdio.h>

enum Err_Asm passes_any_pass(struct Assembler_Processing *asp, int is_second) {
  enum Assembler_Context ctx = ASC_FILE_START;
  char *line = NULL;
  size_t line_len = 0, nl = 1;
  FILE *f = NULL;
  enum Err_Asm err = ASM_NO_ERROR;
  RETURN_IF_FAIL(asp != NULL, ASM_INVALID_ARGS);
  PRINT_VERBOSE("STARTING PASS %i\n", is_second ? 2 : 1);
  RET_VERBOSE_CLN_IF_FAIL(fu_open(asp->config->source, &f, NULL),
                          ASM_CANNOT_OPEN_FILE, "Couldn't open file: %s\n",
                          asp->config->source);

  while (fu_getline(&line, &line_len, f) != -1) {
    if (is_second) {
      REUSE_ERR_IF_FAIL(pass2_line(asp, &ctx, nl, line));
    } else {
      REUSE_ERR_IF_FAIL(_pass1_line(asp, &ctx, nl, line));
    }
    nl++;
  }

cleanup:
  if (f) {
    fclose(f);
    f = NULL;
  }
  if (line) {
    jree(line);
    line = NULL;
  }
  return err;
}

enum Err_Asm passes_line(struct Assembler_Processing *asp,
                         enum Assembler_Context *ctx, size_t nl,
                         const char *line, int is_second) {
  struct Token *tokens = NULL;
  struct Parsed_Statement *pstmt = NULL;
  enum Err_Asm err = ASM_NO_ERROR;

  PRINT_VERBOSE("Tokenizing line.\n");
  tokens = lexer_tokenize_line(line, nl);
  ERR_IF_FAIL(tokens, ASM_CREATING_TOKENS);
  if (asp->config->flag_verbose) {
    print_tokens(tokens);
  }
  PRINT_VERBOSE("Parsing tokens.\n");
  pstmt = convert_parse_tokens(tokens, nl);
  ERR_IF_FAIL(pstmt &&
                  (pstmt->err == PAR_NO_ERROR || pstmt->err == PAR_EMPTY_LINE),
              ASM_CREATING_PSTMT);

  PRINT_VERBOSE("Evaluating parsed statement.\n");
  if (is_second) {
    REUSE_ERR_IF_FAIL(pass2_decide(pstmt, asp, ctx, nl));
  } else {
    REUSE_ERR_IF_FAIL(_pass1_decide(pstmt, asp, ctx, nl));
  }

cleanup:
  if (tokens) {
    lexer_free_tokens(tokens);
    tokens = NULL;
  }
  if (pstmt) {
    p_stmt_free(&pstmt);
  }
  return err;
}
