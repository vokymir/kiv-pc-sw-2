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

#define PRINT_VERBOSE(...)                                                     \
  print_verbose(asp && asp->config && asp->config->flag_verbose, __VA_ARGS__)
#define PRINT_VERBOSE_CLN(...)                                                 \
  print_verbose_clean(asp && asp->config && asp->config->flag_verbose,         \
                      __VA_ARGS__)
#define PRINT_VERBOSE_DBG(...) print_verbose(DEBUG, __VA_ARGS__)

// If condition fail, print verbose clean & return err.
#define RET_VERBOSE_CLN_IF_FAIL(cond, err, ...)                                \
  do {                                                                         \
    if (!(cond)) {                                                             \
      PRINT_VERBOSE_CLN(__VA_ARGS__);                                          \
      return err;                                                              \
    }                                                                          \
  } while (0)

// ===== STATIC HELPER DECLARATIONS =====

// I accidentally created a continuous array of tokens in lexer but require
// array of pointers to tokens in parser. This function is a bridge between
// these differences. Caller must free this "convertor" after is used. Return
// NULL on failure, pointer on success.
static const struct Token **_convert_tokens(const struct Token *orig);

// Free the array of pointers to tokens & set the pointer to null.
// It frees only the array, not the tokens = can have const ptr.
static void _free_tokens_convertor(const struct Token **tokens[]);

// Convert the structure given from lexer (struct Token *) to that wanted in
// parser (struct Token **). Free that structure on return. Otherwise only a
// wrapper around parse_tokens in parser.
static struct Parsed_Statement *_parse_tokens(const struct Token *tokens,
                                              size_t nl);

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

// Given any ASM error, convert it to corresponding MAIN error.
static enum Err_Main _err_convert(enum Err_Asm err);

// ===== HEADER DEFINITIONS =====

enum Err_Main process_assembler(struct Assembler_Processing *asp) {
  enum Err_Asm res = ASM_NO_ERROR;
  if ((res = pass1(asp)) != ASM_NO_ERROR) {
    return _err_convert(res);
  }
  if ((res = pass2(asp)) != ASM_NO_ERROR) {
    return _err_convert(res);
  }

  return ERR_NO_ERROR;
}

enum Err_Asm pass1(struct Assembler_Processing *asp) {
  enum Assembler_Context ctx = ASC_FILE_START;
  char *line = NULL;
  size_t line_len = 0, nl = 1;
  FILE *f = NULL;
  enum Err_Asm err = ASM_NO_ERROR;
  RETURN_IF_FAIL(asp != NULL, ASM_INVALID_ARGS);
  PRINT_VERBOSE("STARTING PASS 1\n");
  if (!fu_open(asp->config->source, &f)) {
    PRINT_VERBOSE("Couldn't open file: %s\n", asp->config->source);
    return ASM_CANNOT_OPEN_FILE;
  }

  while (fu_getline(&line, &line_len, f) != -1) {
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

static const struct Token **_convert_tokens(const struct Token *orig) {
  size_t count = 0, i = 0;
  const struct Token **res = NULL;
  RETURN_IF_FAIL(orig, NULL);

  while (orig[count].type != TOKEN_EOF) {
    count++;
  }
  count++; // also count EOF

  res = jalloc((count + 1) * sizeof(*res)); // +1 for NULL terminator
  RETURN_IF_FAIL(res, NULL);

  for (i = 0; i < count; i++) {
    res[i] = &orig[i];
  }
  res[count] = NULL;

  return res;
}

static void _free_tokens_convertor(const struct Token **tokens[]) {
  if (!tokens || !*tokens) {
    return;
  }
  jree((void *)*tokens);
  *tokens = NULL;
}

static struct Parsed_Statement *_parse_tokens(const struct Token *tokens,
                                              size_t nl) {
  struct Parsed_Statement *pstmt = NULL;
  const struct Token **converted_tokens = NULL;
  RETURN_IF_FAIL(tokens, 0);
  converted_tokens = _convert_tokens(tokens);
  RETURN_IF_FAIL(converted_tokens, NULL);

  pstmt = parse_tokens(converted_tokens, nl);

  if (converted_tokens) {
    // cast out const, because of free
    _free_tokens_convertor(&converted_tokens);
  }
  return pstmt;
}

static enum Err_Asm _pass1_line(struct Assembler_Processing *asp,
                                enum Assembler_Context *ctx, size_t nl,
                                const char *line) {
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
  pstmt = _parse_tokens(tokens, nl);
  ERR_IF_FAIL(pstmt &&
                  (pstmt->err == PAR_NO_ERROR || pstmt->err == PAR_EMPTY_LINE),
              ASM_CREATING_PSTMT);

  PRINT_VERBOSE("Evaluating parsed statement.\n");
  REUSE_ERR_IF_FAIL(_pass1_decide(pstmt, asp, ctx, nl));

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
  PRINT_VERBOSE("Found KMA label on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(asp && asp->config && ctx, ASM_INVALID_ARGS,
                          "but something went WRONG.\n");

  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_FILE_START, ASM_KMA_DOUBLE,
      "resulting in error, because it IS NOT at the start of file.\n");

  *ctx = ASC_AFTER_KMA;
  PRINT_VERBOSE_CLN("which is OK, because its start of file.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_code_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx,
                                        size_t nl) {
  PRINT_VERBOSE("Found CODE SECTION label on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(asp && asp->config && ctx, ASM_INVALID_ARGS,
                          "but something went WRONG.\n");

  RET_VERBOSE_CLN_IF_FAIL(*ctx != ASC_FILE_START, ASM_KMA_EXPECTED,
                          "resulting in error, because it IS at the start of "
                          "file and KMA was expected.\n");

  *ctx = ASC_CODE;
  PRINT_VERBOSE_CLN("which is OK.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_data_section(struct Assembler_Processing *asp,
                                        enum Assembler_Context *ctx,
                                        size_t nl) {
  PRINT_VERBOSE("Found DATA SECTION label on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(asp && asp->config && ctx, ASM_INVALID_ARGS,
                          "but something went WRONG.\n");

  RET_VERBOSE_CLN_IF_FAIL(*ctx != ASC_FILE_START, ASM_KMA_EXPECTED,
                          "resulting in error, because it IS at the start of "
                          "file and KMA was expected.\n");

  *ctx = ASC_DATA;
  PRINT_VERBOSE_CLN("which is OK.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_data_decl(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl) {
  size_t position = SIZE_MAX, size = SIZE_MAX;
  char *identifier = NULL;
  PRINT_VERBOSE("Found DATA DECLARATION on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(pstmt && asp && asp->config && ctx, ASM_INVALID_ARGS,
                          "but something went WRONG.\n");
  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_DATA, ASM_DATA_ABROAD,
      "but that IS NOT in the DATA section, resultion in ERROR.\n");

  size = pstmt->content.data_decl.total_size;
  identifier = pstmt->content.data_decl.identifier;

  PRINT_VERBOSE_CLN("ADVANCING DATASEGMENT of TOTALSIZE=%zu, ", size);
  position = dtsg_advance(asp->dtsg, size);

  RET_VERBOSE_CLN_IF_FAIL(position != SIZE_MAX, ASM_DTSG_CANNOT_ADVANCE,
                          "but when trying to 'reserve' the space in data "
                          "segment, ERROR happened.\n");
  RET_VERBOSE_CLN_IF_FAIL(
      size <= KMA_DTSG_BYTES, ASM_DTSG_TOO_LARGE,
      "but requested size %zu is larger than whole data segment (%zu).\n", size,
      (size_t)KMA_DTSG_BYTES);
  RET_VERBOSE_CLN_IF_FAIL(
      position <= KMA_DTSG_BYTES - size, ASM_DTSG_TOO_LARGE,
      "but data segment overflow: position=%zu size=%zu capacity=%zu.\n",
      position, size, (size_t)KMA_DTSG_BYTES);
  RET_VERBOSE_CLN_IF_FAIL(
      position <= (size_t)UINT32_MAX, ASM_DTSG_TOO_LARGE,
      "but data segment position %zu does not fit into 32-bit address.\n",
      position);

  RET_VERBOSE_CLN_IF_FAIL(
      symtab_find(asp->symtab, identifier) == NULL, ASM_SYMTAB_ALREADY_EXIST,
      "but identifier %s was already used = illegal redeclaration.\n",
      identifier);

  RET_VERBOSE_CLN_IF_FAIL(
      symtab_add(asp->symtab, identifier, (uint32_t)position),
      ASM_SYMTAB_CANNOT_ADD,
      "but identifier %s couldn't be added to the symbol table.\n", identifier);

  PRINT_VERBOSE_CLN("everything is OK.\n");
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_instruction(struct Parsed_Statement *pstmt,
                                       struct Assembler_Processing *asp,
                                       enum Assembler_Context *ctx, size_t nl) {
  size_t size = SIZE_MAX, position = SIZE_MAX;
  PRINT_VERBOSE("Found INSTRUCTION on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(pstmt && asp && asp->config && ctx, ASM_INVALID_ARGS,
                          "but something went WRONG.\n");
  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_CODE, ASM_CODE_ABROAD,
      "but that IS NOT in the CODE section, resultion in ERROR.\n");

  size = instruction_get_encoded_size(pstmt->content.instruction.descriptor);

  RET_VERBOSE_CLN_IF_FAIL(
      size > 0 && size != SIZE_MAX, ASM_INVALID_INSTUCTION,
      "but either the instructions size is 0 or some other error occured.\n");
  PRINT_VERBOSE_CLN("retrieved the size of instruction (%zu), ", size);

  PRINT_VERBOSE_CLN("ADVANCING CODESEGMENT of TOTALSIZE=%zu, ", size);
  position = cdsg_advance(asp->cdsg, size);
  RET_VERBOSE_CLN_IF_FAIL(
      position <= KMA_CDSG_BYTES - size, ASM_CDSG_TOO_LARGE,
      "but code segment overflow: position=%zu size=%zu capacity=%zu.\n",
      position, size, (size_t)KMA_CDSG_BYTES);
  RET_VERBOSE_CLN_IF_FAIL(
      position <= (size_t)UINT32_MAX, ASM_CDSG_TOO_LARGE,
      "but code segment position %zu does not fit into 32-bit address.\n",
      position);

  PRINT_VERBOSE_CLN(
      "and reserved the place in code segment for it, on position %zu\n",
      position);
  return ASM_NO_ERROR; // here no print_instruction, that is in 2nd pass
}

static enum Err_Asm _pass1_label_def(struct Parsed_Statement *pstmt,
                                     struct Assembler_Processing *asp,
                                     enum Assembler_Context *ctx, size_t nl) {
  char *label_name = NULL;
  size_t position = SIZE_MAX;
  PRINT_VERBOSE("Found LABEL definition on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(pstmt && asp && asp->config && ctx, ASM_INVALID_ARGS,
                          "but something went WRONG.\n");
  label_name = pstmt->content.label_def.label_name;
  PRINT_VERBOSE_CLN("the label name is (%s), ", label_name);
  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_CODE, ASM_CODE_ABROAD,
      "but that IS NOT in the CODE section, resultion in ERROR.\n");

  PRINT_VERBOSE_CLN("retrieving label position in code segment, ");
  position = cdsg_advance(asp->cdsg, 0);
  RET_VERBOSE_CLN_IF_FAIL(
      position <= KMA_CDSG_BYTES, ASM_CDSG_TOO_LARGE,
      "but code segment overflow: position=%zu capacity=%zu.\n", position,
      (size_t)KMA_CDSG_BYTES);
  RET_VERBOSE_CLN_IF_FAIL(
      position <= (size_t)UINT32_MAX, ASM_CDSG_TOO_LARGE,
      "but code segment position %zu does not fit into 32-bit address.\n",
      position);

  RET_VERBOSE_CLN_IF_FAIL(
      symtab_find(asp->symtab, label_name) == NULL, ASM_SYMTAB_ALREADY_EXIST,
      "but label name %s was already used = illegal redeclaration.\n",
      label_name);
  RET_VERBOSE_CLN_IF_FAIL(
      symtab_add(asp->symtab, label_name, (uint32_t)position),
      ASM_SYMTAB_CANNOT_ADD,
      "but identifier %s couldn't be added to the symbol table.\n", label_name);

  PRINT_VERBOSE_CLN("and saved its position (%zu) in the symbol table.\n",
                    position);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_none(struct Assembler_Processing *asp, size_t nl) {
  PRINT_VERBOSE(
      "Found NOTHIMG on line %zu, might be an empty line, or only comment.\n",
      nl);
  return ASM_NO_ERROR;
}

static enum Err_Asm _pass1_error(struct Assembler_Processing *asp, size_t nl) {
  PRINT_VERBOSE("Weird line %zu, cannot find known statement.\n", nl);
  return ASM_UNKNOWN_PSTMT_TYPE;
}

static enum Err_Main _err_convert(enum Err_Asm err) { // TODO:
  return ERR_NO_ERROR;
}
