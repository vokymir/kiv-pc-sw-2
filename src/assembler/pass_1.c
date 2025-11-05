#include "assembler.h"
#include "common.h"
#include "internal.h"

#include "assembler_pass_1.h"
#include "assembler_passes.h"

enum Err_Asm pass1_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line) {
  return passes_line(asp, ctx, nl, line, 0);
}

enum Err_Asm pass1_decide(struct Parsed_Statement *pstmt,
                          struct Assembler_Processing *asp,
                          enum Assembler_Context *ctx, size_t nl) {
  RET_PRINT_ERR_IF_FAIL_ARGS(pstmt, asp, ctx);

  switch (pstmt->type) {
  case STMT_KMA:
    return passes_kma(asp, ctx, nl);
  case STMT_SECTION_CODE:
    return passes_code_section(asp, ctx, nl);
  case STMT_SECTION_DATA:
    return passes_data_section(asp, ctx, nl);
  case STMT_DATA_DECL:
    return pass1_data_decl(pstmt, asp, ctx, nl);
  case STMT_INSTRUCTION:
    return pass1_instruction(pstmt, asp, ctx, nl);
  case STMT_LABEL_DEF:
    return pass1_label_def(pstmt, asp, ctx, nl);
  case STMT_NONE:
    return passes_none(asp, nl);
  case STMT_ERROR:
  default:
    return passes_error(asp, nl);
  }
}

enum Err_Asm pass1_data_decl(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl) {
  size_t position = SIZE_MAX, size = SIZE_MAX;
  char *identifier = NULL;
  PRINT_VERBOSE("Found DATA DECLARATION on line %zu, ", nl);

  IF_FAIL(pstmt && asp && asp->config && ctx) {
    PRINT_VERBOSE("but something went WRONG.\n");
    PRINT_ERR("Invalid arguments.");
    return ASM_INVALID_ARGS;
  }

  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_DATA, ASM_DATA_ABROAD,
      "but that IS NOT in the DATA section, resulting in ERROR.\n");

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

enum Err_Asm pass1_instruction(struct Parsed_Statement *pstmt,
                               struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl) {
  size_t size = SIZE_MAX, position = SIZE_MAX;
  PRINT_VERBOSE("Found INSTRUCTION on line %zu, ", nl);

  IF_FAIL(pstmt && asp && asp->config && ctx) {
    PRINT_VERBOSE_CLN("but something went WRONG.\n");
    PRINT_ERR("Invalid arguments.");
    return ASM_INVALID_ARGS;
  }

  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_CODE, ASM_CODE_ABROAD,
      "but that IS NOT in the CODE section, resulting in ERROR.\n");

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

enum Err_Asm pass1_label_def(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl) {
  char *label_name = NULL;
  size_t position = SIZE_MAX;
  PRINT_VERBOSE("Found LABEL definition on line %zu, ", nl);

  IF_FAIL(pstmt && asp && asp->config && ctx) {
    PRINT_VERBOSE_CLN("but something went WRONG.\n");
    PRINT_ERR("Invalid arguments.");
    return ASM_INVALID_ARGS;
  }

  label_name = pstmt->content.label_def.label_name;
  PRINT_VERBOSE_CLN("the label name is (%s), ", label_name);
  RET_VERBOSE_CLN_IF_FAIL(
      *ctx == ASC_CODE, ASM_CODE_ABROAD,
      "but that IS NOT in the CODE section, resulting in ERROR.\n");

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
