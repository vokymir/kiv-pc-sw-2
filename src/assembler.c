#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "assembler.h"
#include "codeseg.h"
#include "common.h"
#include "dataseg.h"
#include "fileutil.h"
#include "instruction.h"
#include "lexer.h"
#include "memory.h"
#include "parser.h"
#include "symbol.h"

// If condition fail, set variable 'err' to given er
// & goto cleanup.
#define ERR_IF_FAIL(cond, er)                                                  \
  do {                                                                         \
    if (!(cond)) {                                                             \
      err = (er);                                                              \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

// Check if func == ASM_NO_ERROR
// Reuse error given from func & goto cleanup
#define REUSE_ERR_IF_FAIL(func)                                                \
  do {                                                                         \
    if ((err = (func)) != ASM_NO_ERROR) {                                      \
      goto cleanup;                                                            \
    }                                                                          \
  } while (0)

// ===== STATIC HELPER DECLARATIONS =====

// Process one line in the first pass of the assembler code.
// Return adequate error code, edit context if changed.
static enum Err_Asm _pass1_line(struct Assembler_Processing *asp,
                                enum Assembler_Context *ctx, size_t nl,
                                const char *line);

// Decide what to do in pass1 with given pstmt in pass one.
static enum Err_Asm _pass1_decide(struct Parsed_Statement *pstmt,
                                  struct Assembler_Processing *asp,
                                  enum Assembler_Context *ctx, size_t nl);

// Perform all actions required for first pass, case STMT_KMA.
// Return adequate err_asm, preferably asm_no_error
static enum Err_Asm _pass1_kma(struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_code_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_data_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_data_decl(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_instruction(struct Parsed_Statement *pstmt,
                                       struct Assembler_Processing *asp,
                                       enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_label_def(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_none(struct Assembler_Processing *asp,
                                enum Assembler_Context *ctx, size_t nl);

static enum Err_Asm _pass1_error(struct Assembler_Processing *asp,
                                 enum Assembler_Context *ctx, size_t nl);

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
  RETURN_IF_FAIL(asp, ASM_INVALID_ARGS);
  RETURN_IF_FAIL(fu_open(asp->config->source, &f), ASM_CANNOT_OPEN_FILE);

  while (fu_getline(&line, &line_len, f)) {
    REUSE_ERR_IF_FAIL(_pass1_line(asp, &ctx, nl, line));
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

// TODO:
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
    asp_free(&asp);
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
    dtsg_free(&asp->dtsg);
  }
  if (asp->cdsg) {
    cdsg_free(&asp->cdsg);
  }
}

void asp_free(struct Assembler_Processing **asp) {
  if (!asp || !*asp) {
    return;
  }
  asp_deinit(*asp);
  jree(*asp);
  *asp = NULL;
}

// ===== STATIC HELPER DEFINITIONS =====

static enum Err_Asm _pass1_line(struct Assembler_Processing *asp,
                                enum Assembler_Context *ctx, size_t nl,
                                const char *line) {
  struct Token *tokens = NULL;
  const struct Token *ctokens = NULL;
  struct Parsed_Statement *pstmt = NULL;
  enum Err_Asm err = ASM_NO_ERROR;

  tokens = lexer_tokenize_line(line, nl);
  ERR_IF_FAIL(tokens, ASM_CREATING_TOKENS);
  ctokens = tokens;
  pstmt = parse_tokens(&ctokens, nl);
  ERR_IF_FAIL(pstmt && pstmt->err == PAR_NO_ERROR, ASM_CREATING_PSTMT);

  REUSE_ERR_IF_FAIL(_pass1_decide(pstmt, asp, ctx, nl));

cleanup:
  if (tokens) {
    lexer_free_tokens(tokens);
    tokens = NULL;
    ctokens = NULL;
  }
  if (pstmt) {
    p_stmt_free(&pstmt);
  }
  return err;
}

static enum Err_Asm _pass1_decide(struct Parsed_Statement *pstmt,
                                  struct Assembler_Processing *asp,
                                  enum Assembler_Context *ctx, size_t nl) {
  size_t pos = SIZE_MAX, size = SIZE_MAX;
  RETURN_IF_FAIL(pstmt && asp && ctx, ASM_INVALID_ARGS);

  switch (pstmt->type) {
  case STMT_KMA:
    return _pass1_kma(asp, ctx, nl);
  case STMT_SECTION_CODE:
    return _pass1_code_section(asp, ctx, nl);
  case STMT_SECTION_DATA:
    return _pass1_data_section(asp, ctx, nl);
  case STMT_DATA_DECL:
    return _pass1_data_decl(pstmt, asp, ctx, nl);
  case STMT_INSTRUCTION:
    RETURN_IF_FAIL(*ctx == ASC_CODE, ASM_CODE_ABROAD);
    size = instruction_get_encoded_size(pstmt->content.instruction.descriptor);
    RETURN_IF_FAIL(size > 0 && size != SIZE_MAX, ASM_INVALID_INSTUCTION);
    pos = cdsg_advance(asp->cdsg, size);
    RETURN_IF_FAIL(pos != SIZE_MAX, ASM_CDSG_CANNOT_ADVANCE);
    break;
  case STMT_LABEL_DEF:
    RETURN_IF_FAIL(*ctx == ASC_CODE, ASM_CODE_ABROAD);
    pos = cdsg_advance(asp->cdsg, 0);
    RETURN_IF_FAIL(pos != SIZE_MAX, ASM_CDSG_CANNOT_ADVANCE);
    RETURN_IF_FAIL(
        symtab_find(asp->symtab, pstmt->content.label_def.label_name) == NULL,
        ASM_SYMTAB_ALREADY_EXIST);
    RETURN_IF_FAIL(pos <= UINT32_MAX, ASM_SYMTAB_DATA_TOO_LARGE);
    RETURN_IF_FAIL(symtab_add(asp->symtab, pstmt->content.label_def.label_name,
                              (uint32_t)pos),
                   ASM_SYMTAB_CANNOT_ADD);
    break;
  case STMT_NONE:
    break;
  case STMT_ERROR:
  default:
    return ASM_UNKNOWN_PSTMT_TYPE;
    break;
  }

  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_kma(struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl) {
  if (!asp || !asp->config || !*ctx) {
    print_verbose(asp->config->flag_verbose,
                  "Found KMA label on line %zu, but something went WRONG.", nl);
    return ASM_INVALID_ARGS;
  }

  if (*ctx != ASC_FILE_START) {
    print_verbose(asp->config->flag_verbose,
                  "Found KMA label on line %zu, resulting in error, because it "
                  "IS NOT at the start of file.",
                  nl);
    return ASM_KMA_DOUBLE;
  }

  *ctx = ASC_AFTER_KMA;
  print_verbose(asp->config->flag_verbose,
                "Found KMA label on line %zu, which is OK, because its "
                "start of file.",
                nl);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_code_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx,
                                        size_t nl) {
  if (!asp || !asp->config || !*ctx) {
    print_verbose(
        asp->config->flag_verbose,
        "Found CODE SECTION label on line %zu, but something went WRONG.", nl);
    return ASM_INVALID_ARGS;
  }

  if (*ctx == ASC_FILE_START) {
    print_verbose(asp->config->flag_verbose,
                  "Found CODE SECTION label on line %zu, resulting in error, "
                  "because it IS at the start of file and KMA was expected.",
                  nl);
    return ASM_KMA_EXPECTED;
  }

  *ctx = ASC_CODE;
  print_verbose(asp->config->flag_verbose,
                "Found CODE SECTION label on line %zu, which is OK.", nl);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_data_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx,
                                        size_t nl) {
  if (!asp || !asp->config || !*ctx) {
    print_verbose(
        asp->config->flag_verbose,
        "Found DATA SECTION label on line %zu, but something went WRONG.", nl);
    return ASM_INVALID_ARGS;
  }

  if (*ctx == ASC_FILE_START) {
    print_verbose(asp->config->flag_verbose,
                  "Found DATA SECTION label on line %zu, resulting in error, "
                  "because it IS at the start of file and KMA was expected.",
                  nl);
    return ASM_KMA_EXPECTED;
  }

  *ctx = ASC_DATA;
  print_verbose(asp->config->flag_verbose,
                "Found DATA SECTION label on line %zu, which is OK.", nl);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_data_decl(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl) {
  size_t position = SIZE_MAX;
  if (!pstmt || !asp || !asp->config || !*ctx) {
    print_verbose(
        asp->config->flag_verbose,
        "Found DATA DECLARATION on line %zu, but something went WRONG.", nl);
    return ASM_INVALID_ARGS;
  }

  if (*ctx != ASC_DATA) {
    print_verbose(asp->config->flag_verbose,
                  "Found DATA DECLARATION on line %zu, but that IS NOT in the "
                  "data section, resultion in ERROR.");
    return ASM_DATA_ABROAD;
  }

  position = dtsg_advance(asp->dtsg, pstmt->content.data_decl.total_size);

  if (position == SIZE_MAX) {
    print_verbose(asp->config->flag_verbose,
                  "Found DATA DECLARATION on line %zu, but when trying to "
                  "'reserve' the space in data segment, ERROR happened.",
                  nl);
    return ASM_DTSG_CANNOT_ADVANCE;
  }

  if (position > UINT32_MAX) {
    print_verbose(asp->config->flag_verbose,
                  "Found DATA DECLARATION on line %zu, but data segment is "
                  "definitely too large for KMA computer (%zu). ",
                  nl, position);
    return ASM_SYMTAB_DATA_TOO_LARGE;
  }

  if (symtab_find(asp->symtab, pstmt->content.data_decl.identifier) != NULL) {
    print_verbose(asp->config->flag_verbose,
                  "Found DATA DECLARATION on line %zu, but identifier %s was "
                  "already used = illegal redeclaration.",
                  nl, pstmt->content.data_decl.identifier);
    return ASM_SYMTAB_ALREADY_EXIST;
  }

  if (!symtab_add(asp->symtab, pstmt->content.data_decl.identifier,
                  (uint32_t)position)) {
    print_verbose(asp->config->flag_verbose,
                  "Found DATA DECLARATION on line %zu, but identifier %s "
                  "couldn't be added to the symbol table.",
                  nl, pstmt->content.data_decl.identifier);
    return ASM_SYMTAB_CANNOT_ADD;
  }

  return ASM_NO_ERROR;
}
