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

#define PRINT_VERBOSE(...) print_verbose(asp->config->flag_verbose, __VA_ARGS__)

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

static enum Err_Asm _pass1_none(struct Assembler_Processing *asp, size_t nl);

static enum Err_Asm _pass1_error(struct Assembler_Processing *asp, size_t nl);

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
  print_verbose(1, "Pass1 before any checks.\n");
  RETURN_IF_FAIL(asp, ASM_INVALID_ARGS);
  PRINT_VERBOSE("Starting pass1...\n");
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
    return _pass1_instruction(pstmt, asp, ctx, nl);
  case STMT_LABEL_DEF:
    return _pass1_label_def(pstmt, asp, ctx, nl);
  case STMT_NONE:
    return _pass1_none(asp, nl);
  case STMT_ERROR:
  default:
    return _pass1_error(asp, nl);
  }
}

static enum Err_Asm _pass1_kma(struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl) {
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose, "Found KMA label on line %zu, ", nl);
  if (!*ctx) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but something went WRONG.\n");
    return ASM_INVALID_ARGS;
  }

  if (*ctx != ASC_FILE_START) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "resulting in error, because it IS NOT at the start of file.\n");
    return ASM_KMA_DOUBLE;
  }

  *ctx = ASC_AFTER_KMA;
  print_verbose_clean(asp->config->flag_verbose,
                      "which is OK, because its start of file.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_code_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx,
                                        size_t nl) {
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose,
                "Found CODE SECTION label on line %zu, ", nl);
  if (!*ctx) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but something went WRONG.\n");
    return ASM_INVALID_ARGS;
  }

  if (*ctx == ASC_FILE_START) {
    print_verbose_clean(asp->config->flag_verbose,
                        "resulting in error, because it IS at the start of "
                        "file and KMA was expected.\n");
    return ASM_KMA_EXPECTED;
  }

  *ctx = ASC_CODE;
  print_verbose_clean(asp->config->flag_verbose, "which is OK.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_data_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx,
                                        size_t nl) {
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose,
                "Found DATA SECTION label on line %zu, ", nl);
  if (!*ctx) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but something went WRONG.\n");
    return ASM_INVALID_ARGS;
  }

  if (*ctx == ASC_FILE_START) {
    print_verbose_clean(asp->config->flag_verbose,
                        "resulting in error, because it IS at the start of "
                        "file and KMA was expected.\n");
    return ASM_KMA_EXPECTED;
  }

  *ctx = ASC_DATA;
  print_verbose_clean(asp->config->flag_verbose, "which is OK.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_data_decl(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl) {
  size_t position = SIZE_MAX;
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose,
                "Found DATA DECLARATION on line %zu, ", nl);
  if (!pstmt || !*ctx) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but something went wrong.\n");
    return ASM_INVALID_ARGS;
  }

  if (*ctx != ASC_DATA) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but that IS NOT in the data section, resultion in ERROR.\n");
    return ASM_DATA_ABROAD;
  }

  position = dtsg_advance(asp->dtsg, pstmt->content.data_decl.total_size);

  if (position == SIZE_MAX) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but when trying to 'reserve' the space in data "
                        "segment, ERROR happened.\n");
    return ASM_DTSG_CANNOT_ADVANCE;
  }

  if (position > UINT32_MAX || position >= KMA_DTSG_BYTES) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but data segment is too large for KMA computer (%zu).\n", position);
    return ASM_DTSG_TOO_LARGE;
  }

  if (symtab_find(asp->symtab, pstmt->content.data_decl.identifier) != NULL) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but identifier %s was already used = illegal redeclaration.\n",
        pstmt->content.data_decl.identifier);
    return ASM_SYMTAB_ALREADY_EXIST;
  }

  if (!symtab_add(asp->symtab, pstmt->content.data_decl.identifier,
                  (uint32_t)position)) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but identifier %s couldn't be added to the symbol table.\n",
        pstmt->content.data_decl.identifier);
    return ASM_SYMTAB_CANNOT_ADD;
  }

  print_verbose_clean(asp->config->flag_verbose, "everything is OK.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_instruction(struct Parsed_Statement *pstmt,
                                       struct Assembler_Processing *asp,
                                       enum Assembler_Context *ctx, size_t nl) {
  size_t size = SIZE_MAX, position = SIZE_MAX;
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose, "Found INSTRUCTION on line %zu, ",
                nl);
  if (!*ctx) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but something went WRONG.\n");
    return ASM_INVALID_ARGS;
  }

  if (*ctx != ASC_CODE) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but that IS NOT in the code section, resultion in ERROR.\n");
    return ASM_CODE_ABROAD;
  }

  size = instruction_get_encoded_size(pstmt->content.instruction.descriptor);
  if (size == 0 || size == SIZE_MAX) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but either the instructions size is 0 or some other error occured.\n");
    return ASM_INVALID_INSTUCTION;
  }
  print_verbose_clean(asp->config->flag_verbose,
                      "retrieved the size of instruction (%zu), ", size);

  position = cdsg_advance(asp->cdsg, size);
  if (position == SIZE_MAX) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but cannot advance in code segment, ERROR.\n");
    return ASM_CDSG_CANNOT_ADVANCE;
  }

  if (position > UINT32_MAX || position >= KMA_CDSG_BYTES) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but code segment is too large for KMA computer (%zu).\n", position);
    return ASM_CDSG_TOO_LARGE;
  }

  print_verbose_clean(
      asp->config->flag_verbose,
      "and reserved the place in code segment for it, on position %zu\n",
      position);
  return ASM_NO_ERROR; // here no print_instruction, that is in 2nd pass
}

static enum Err_Asm _pass1_label_def(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl) {
  size_t position = SIZE_MAX;
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose,
                "Found LABEL definition on line %zu, ", nl);
  if (!*ctx) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but something went WRONG.\n");
    return ASM_INVALID_ARGS;
  }
  print_verbose_clean(asp->config->flag_verbose, "the label name is (%s), ",
                      pstmt->content.label_def.label_name);

  if (*ctx != ASC_CODE) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but that IS NOT in the code section, resultion in ERROR.\n");
    return ASM_CODE_ABROAD;
  }

  position = cdsg_advance(asp->cdsg, 0);
  if (position == SIZE_MAX) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but cannot advance in code segment, ERROR.\n");
    return ASM_CDSG_CANNOT_ADVANCE;
  }

  if (position > UINT32_MAX || position >= KMA_CDSG_BYTES) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but code segment is too large for KMA computer (%zu).\n", position);
    return ASM_CDSG_TOO_LARGE;
  }

  if (symtab_find(asp->symtab, pstmt->content.label_def.label_name) != NULL) {
    print_verbose_clean(asp->config->flag_verbose,
                        "but the label already exist.\n");
    return ASM_SYMTAB_ALREADY_EXIST;
  }

  if (!symtab_add(asp->symtab, pstmt->content.label_def.label_name,
                  (uint32_t)position)) {
    print_verbose_clean(
        asp->config->flag_verbose,
        "but the label couldn't be added to the symbol table.\n");
    return ASM_SYMTAB_CANNOT_ADD;
  }

  print_verbose_clean(asp->config->flag_verbose,
                      "and saved its position (%zu) in the symbol table.",
                      position);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_none(struct Assembler_Processing *asp, size_t nl) {
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(
      asp->config->flag_verbose,
      "Found NOTHIMG on line %zu, might be an empty line, or only comment.\n",
      nl);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_error(struct Assembler_Processing *asp, size_t nl) {
  if (!asp || !asp->config) {
    return ASM_INVALID_ARGS;
  }
  print_verbose(asp->config->flag_verbose,
                "Weird line %zu, cannot find known statement.\n", nl);
  return ASM_UNKNOWN_PSTMT_TYPE;
}
