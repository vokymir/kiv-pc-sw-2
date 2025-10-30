#include "assembler.h"
#include "codeseg.h"
#include "common.h"
#include "dataseg.h"
#include "fileutil.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "symbol.h"
#include <stddef.h>
#include <stdio.h>

#define ERR_IF_FAIL(cond, er)                                                  \
  err = (er);                                                                  \
  CLEANUP_IF_FAIL(cond);

// ===== STATIC HELPER DECLARATIONS =====

// Process one line in the first pass of the assembler code.
// Return adequate error code, edit context if changed.
static enum Err_Asm _pass1_line(const char *line, size_t nl,
                                enum Assembler_Context *ctx);

// ===== HEADER DEFINITIONS =====

enum Err_Main process_assembler(struct Assembler_Processing *asp) {
  enum Err_Asm res = ASM_NO_ERROR;
  if ((res = pass1(asp)) != ASM_NO_ERROR) {
    return ERR_SYNTAX_ERROR; // what else
  }
  if ((res = pass2(asp)) != ASM_NO_ERROR) {
    return ERR_SYNTAX_ERROR; // what else
  }

  return ERR_NO_ERROR;
}

enum Err_Asm pass1(struct Assembler_Processing *asp) {
  enum Assembler_Context ctx = ASC_FILE_START;
  char *line = NULL;
  size_t line_len = 0, nl = 1;
  FILE *f = NULL;
  enum Err_Asm err = ASM_NO_ERROR;
  struct Token *tokens = NULL;        // because cannot free const
  const struct Token *ctokens = NULL; // but parse_tokens needs const
  struct Parsed_Statement *pstmt = NULL;
  RETURN_IF_FAIL(asp, ASM_INVALID_ASP);
  RETURN_IF_FAIL(fu_open(asp->config->source, &f), ASM_CANNOT_OPEN_FILE);

  while (fu_getline(&line, &line_len, f)) {
    tokens = lexer_tokenize_line(line, nl);
    ERR_IF_FAIL(tokens, ASM_CREATING_TOKENS);
    ctokens = tokens;
    pstmt = parse_tokens(&ctokens, nl);
    ERR_IF_FAIL(pstmt && pstmt->err == PAR_NO_ERROR, ASM_CREATING_PSTMT);

    switch (pstmt->type) {
    case STMT_KMA:
      ERR_IF_FAIL(nl == 1, ASM_KMA_IN_THE_MIDDLE);
      break;
    case STMT_NONE:
      break;
    case STMT_SECTION_CODE:
      ctx = ASC_CODE;
      break;
    case STMT_SECTION_DATA:
      ctx = ASC_DATA;
      break;
    case STMT_DATA_DECL:
      ERR_IF_FAIL(ctx == ASC_DATA, ASM_DATA_ABROAD);
      // dtsg advance
      // symtab define
      break;
    case STMT_INSTRUCTION:
      ERR_IF_FAIL(ctx == ASC_CODE, ASM_CODE_ABROAD);
      // cdsg advance
      break;
    case STMT_LABEL_DEF:
      // symtab define
      break;
    case STMT_ERROR:
    default:
      err = ASM_UNKNOWN_PSTMT_TYPE;
      goto cleanup;
      break;
    }
    // enforce .KMA on the start of file
    if (nl == 1 && pstmt->type != STMT_KMA) {
      err = ASM_MISSING_KMA;
      goto cleanup;
    }

    lexer_free_tokens(tokens);
    p_stmt_free(&pstmt);
    nl++;
  }

cleanup:
  if (tokens) {
    lexer_free_tokens(tokens);
  }
  if (pstmt) {
    p_stmt_free(&pstmt);
  }
  return err;
}

enum Err_Asm pass2(struct Assembler_Processing *asp) { return asp == 1; }

struct Assembler_Processing *asp_create(const struct Config *config,
                                        struct Symbol_Table *symtab,
                                        struct Data_Segment *dtsg,
                                        struct Code_Segment *cdsg) {
  struct Assembler_Processing *asp =
      jalloc(sizeof(struct Assembler_Processing));
  if (!asp) {
    return NULL;
  }
  if (!asp_init(asp, config, symtab, dtsg, cdsg)) {
    jree(asp);
    return NULL;
  }
  return asp;
}

int asp_init(struct Assembler_Processing *asp, const struct Config *config,
             struct Symbol_Table *symtab, struct Data_Segment *dtsg,
             struct Code_Segment *cdsg) {
  if (!asp) {
    return 0;
  }
  asp->config = config;

  if (symtab) {
    asp->symtab = symtab;
  } else {
    asp->symtab = symtab_create();
  }
  CLEANUP_IF_FAIL(asp->symtab);

  if (dtsg) {
    asp->dtsg = dtsg;
  } else {
    asp->dtsg = dtsg_create();
  }
  CLEANUP_IF_FAIL(asp->dtsg);

  if (cdsg) {
    asp->cdsg = cdsg;
  } else {
    asp->cdsg = cdsg_create();
  }
  CLEANUP_IF_FAIL(asp->cdsg);

  return 1;

cleanup:
  asp_deinit(asp);
  return 0;
}

void asp_deinit(struct Assembler_Processing *asp) {
  if (!asp) {
    return;
  }
  asp->config = NULL;
  if (asp->symtab) {
    symtab_free(&asp->symtab);
  }
  if (asp->dtsg) {
    dtsg_free(asp->dtsg);
    asp->dtsg = NULL;
  }
  if (asp->cdsg) {
    cdsg_free(asp->cdsg);
    asp->cdsg = NULL;
  }
}

void asp_free(struct Assembler_Processing **asp) {
  if (!asp || !*asp) {
    return;
  }
  asp_deinit(*asp);
  jree(asp);
}

// ===== STATIC HELPER DEFINITIONS =====

static enum Err_Asm _pass1_line(const char *line, size_t nl,
                                enum Assembler_Context *ctx);
