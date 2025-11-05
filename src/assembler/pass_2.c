#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "instruction.h"
#include "internal.h"
#include "parser.h"

#include "assembler_convert.h"
#include "assembler_pass_2.h"
#include "assembler_passes.h"

enum Err_Asm pass2_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line) {
  return passes_line(asp, ctx, nl, line, 1);
}

enum Err_Asm pass2_decide(struct Parsed_Statement *pstmt,
                          struct Assembler_Processing *asp,
                          enum Assembler_Context *ctx, size_t nl) {
  RETURN_IF_FAIL(pstmt && asp && ctx, ASM_INVALID_ARGS);

  switch (pstmt->type) {
  case STMT_KMA:
    return passes_kma(asp, ctx, nl);
  case STMT_SECTION_CODE:
    return passes_code_section(asp, ctx, nl);
  case STMT_SECTION_DATA:
    return passes_data_section(asp, ctx, nl);
  case STMT_DATA_DECL:
    return pass2_data_decl(pstmt, asp, ctx, nl);
  case STMT_INSTRUCTION:
    return pass2_instruction(pstmt, asp, ctx, nl);
  case STMT_LABEL_DEF:
    return ASM_NO_ERROR; // label definition belongs to 1st pass
  case STMT_NONE:
    return passes_none(asp, nl);
  case STMT_ERROR:
  default:
    return passes_error(asp, nl);
  }
}

enum Err_Asm pass2_data_decl(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl) {
  size_t i = 0;
  const struct Data_Declaration *dd = NULL;
  const struct Init_Segment *is = NULL;
  enum Err_Asm err = ASM_NO_ERROR;
  PRINT_VERBOSE("Found DATA DECLARATION on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(pstmt && (dd = &pstmt->content.data_decl) &&
                              dd->segments && asp && asp->config && ctx,
                          ASM_INVALID_ARGS, "but something went WRONG.\n");

  // for all segments save them into datasegment
  for (i = 0; i < dd->segment_count; i++) {
    is = &dd->segments[i];
    switch (is->type) {
    case INIT_SEG_UNINIT:
      REUSE_ERR_IF_FAIL(pass2_data_decl_uninit(asp, is));
      break;
    case INIT_SEG_VALUE:
      REUSE_ERR_IF_FAIL(pass2_data_decl_value(asp, is, dd->type));
      break;
    case INIT_SEG_STRING:
      REUSE_ERR_IF_FAIL(pass2_data_decl_string(asp, is));
      break;
    case INIT_SEG_DUP:
      REUSE_ERR_IF_FAIL(pass2_data_decl_dup(asp, is, dd->type));
      break;
    default:
      PRINT_VERBOSE_CLN("but the segment is of UNKNOWN type!\n");
      return ASM_UNKNOWN_INIT_SEG;
    }
  }

  PRINT_VERBOSE_CLN("the declaration was a success.\n");
cleanup:
  return err;
}

enum Err_Asm pass2_instruction(struct Parsed_Statement *pstmt,
                               struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl) {
  size_t addr = SIZE_MAX;
  union op_value op_values[KMA_MAX_OPERANDS] = {0};
  enum Err_Asm err = ASM_NO_ERROR;

  PRINT_VERBOSE("Found INSTRUCTION on line %zu, ", nl);
  RET_VERBOSE_CLN_IF_FAIL(pstmt && asp && ctx, ASM_INVALID_ARGS,
                          "but something went wrong.");
  REUSE_ERR_IF_FAIL(
      pass2_instruction_ops_get(asp, &pstmt->content.instruction, &op_values));

  addr = cdsg_get_size(asp->cdsg);

  if (instruction_is_relative_jump(pstmt->content.instruction.descriptor)) {
    op_values[0].i32 =
        op_values[0].i32 -
        (convert_size_t(addr) + convert_size_t(instruction_get_encoded_size(
                                    pstmt->content.instruction.descriptor)));
    // Relative offset = label address - (instruction address +
    // size of (instruction))
  }

  REUSE_ERR_IF_FAIL(
      pass_2_instruction_ops_set(asp, &pstmt->content.instruction, &op_values));

  PRINT_VERBOSE_CLN(
      "and everything went great (see line below if -i is set).\n");
  print_instruction(asp->config->flag_instruction, nl,
                    &pstmt->content.instruction, addr);

cleanup:
  return err;
}

enum Err_Asm pass2_data_decl_uninit(struct Assembler_Processing *asp,
                                    const struct Init_Segment *is) {
  RETURN_IF_FAIL(asp && asp->dtsg && is, ASM_INVALID_ARGS);

  RET_VERBOSE_CLN_IF_FAIL(
      dtsg_app_zs(asp->dtsg, is->element_count), ASM_DTSG_CANNOT_APPEND,
      "but couldn't append %zu UNINITIALIZED bytes to data segment.\n",
      is->element_count);

  PRINT_VERBOSE_CLN("appended %zu UNINITIALIZED bytes to data segment, ",
                    is->element_count);
  return ASM_NO_ERROR;
}

enum Err_Asm pass2_data_decl_value(struct Assembler_Processing *asp,
                                   const struct Init_Segment *is,
                                   enum Data_Type dt) {
  uint8_t byte = 0;
  RETURN_IF_FAIL(asp && asp->dtsg && is, ASM_INVALID_ARGS);

  if (dt == DATA_BYTE) {
    byte = (uint8_t)(is->data.value & 0xFF);
    RET_VERBOSE_CLN_IF_FAIL(
        dtsg_app_b(asp->dtsg, byte), ASM_DTSG_CANNOT_APPEND,
        "but couldn't append byte 0x%02X to data segment.\n", byte);
    PRINT_VERBOSE_CLN("appended byte 0x%02X to data segment, ", byte);
  } else if (dt == DATA_DWORD) {
    RET_VERBOSE_CLN_IF_FAIL(
        dtsg_app_dw(asp->dtsg, is->data.value), ASM_DTSG_CANNOT_APPEND,
        "but couldn't append %d to data segment.\n", is->data.value);
    PRINT_VERBOSE_CLN("appended DOUBLE WORD %i to data segment, ",
                      is->data.value);
  }

  return ASM_NO_ERROR;
}

enum Err_Asm pass2_data_decl_string(struct Assembler_Processing *asp,
                                    const struct Init_Segment *is) {
  RETURN_IF_FAIL(asp && asp->dtsg && is, ASM_INVALID_ARGS);

  RET_VERBOSE_CLN_IF_FAIL(
      dtsg_app_str(asp->dtsg, is->data.string), ASM_DTSG_CANNOT_APPEND,
      "but couldn't append string '%s' to data segment.\n", is->data.string);
  PRINT_VERBOSE_CLN("appended string '%s' to data segment, ", is->data.string);

  return ASM_NO_ERROR;
}

enum Err_Asm pass2_data_decl_dup(struct Assembler_Processing *asp,
                                 const struct Init_Segment *is,
                                 enum Data_Type dt) {
  uint8_t byte = 0;
  RETURN_IF_FAIL(asp && asp->dtsg && is, ASM_INVALID_ARGS);

  if (dt == DATA_BYTE) {
    byte = (uint8_t)(is->data.dup.value & 0xFF);
    RET_VERBOSE_CLN_IF_FAIL(
        dtsg_app_b_n(asp->dtsg, byte, is->data.dup.count),
        ASM_DTSG_CANNOT_APPEND,
        "but couldn't append %zu times byte 0x%02X to data segment.\n",
        is->data.dup.count, byte);
    PRINT_VERBOSE_CLN("appended %zu times byte 0x%02X to data segment, ",
                      is->data.dup.count, byte);
  } else if (dt == DATA_DWORD) {
    RET_VERBOSE_CLN_IF_FAIL(
        dtsg_app_dw_n(asp->dtsg, is->data.dup.value, is->data.dup.count),
        ASM_DTSG_CANNOT_APPEND,
        "but couldn't append %zu times DWord %i to data segment.\n",
        is->data.dup.count, is->data.dup.value);
    PRINT_VERBOSE_CLN("appended %zu times DWord %i to data segment, ",
                      is->data.dup.count, is->data.dup.value);
  }

  return ASM_NO_ERROR;
}

enum Err_Asm
pass2_instruction_ops_get(struct Assembler_Processing *asp,
                          const struct Instruction_Statement *is,
                          union op_value (*op_values)[KMA_MAX_OPERANDS]) {
  int i = 0;
  enum Err_Asm err = ASM_NO_ERROR;
  RET_VERBOSE_CLN_IF_FAIL(asp && is && op_values, ASM_INVALID_ARGS,
                          "but something went wrong.");

  for (i = 0; i < is->operand_count; i++) {
    REUSE_ERR_IF_FAIL(pass2_intstruction_get_op(asp, is, &(*op_values)[i], i));
  }

cleanup:
  return err;
}

enum Err_Asm pass2_intstruction_get_op(struct Assembler_Processing *asp,
                                       const struct Instruction_Statement *is,
                                       union op_value *op_value, int idx) {
  RET_VERBOSE_CLN_IF_FAIL(asp && is && op_value, ASM_INVALID_ARGS,
                          "but something went wrong.");

  switch (is->operands[idx].type) {
  case OP_REG:
    return pass2_instruction_get_op_reg(
        asp, is->operands[idx].value.register_name, &op_value->ui8);
  case OP_IMM32:
    switch (is->operands[idx].specifier) {
    case OPS_LABEL:
      return pass2_instruction_get_op_label(asp, is->operands[idx].value.label,
                                            &op_value->i32);
    case OPS_OFFSET: // label & variables are saved in the same table
      return pass2_instruction_get_op_label(asp, is->operands[idx].value.label,
                                            &op_value->i32);
    case OPS_NONE:
    default:
      op_value->i32 = is->operands[idx].value.immediate_value;
      break;
    }
    break;
  case OP_NONE:
  default:
    break; // WARN: what else to do? this should run anyway
  }
  return ASM_NO_ERROR;
}

enum Err_Asm pass2_instruction_get_op_reg(struct Assembler_Processing *asp,
                                          const char *reg_name,
                                          uint8_t *value) {
  RET_VERBOSE_CLN_IF_FAIL(asp && reg_name && value, ASM_INVALID_ARGS,
                          "but something went wrong.");

  RET_VERBOSE_CLN_IF_FAIL(
      pass2_register_name_to_value(reg_name, value),
      ASM_INVALID_OPERAND_REGISTER,
      "but the register name '%s' is not a valid register name.\n", reg_name);

  return ASM_NO_ERROR;
}

enum Err_Asm pass2_instruction_get_op_label(struct Assembler_Processing *asp,
                                            const char *lab_name,
                                            int32_t *value) {
  struct Symbol *s = NULL;
  RET_VERBOSE_CLN_IF_FAIL(asp && lab_name && value, ASM_INVALID_ARGS,
                          "but something went wrong.");

  s = symtab_find(asp->symtab, lab_name);
  RET_VERBOSE_CLN_IF_FAIL(s, ASM_INVALID_OPERAND_LABEL,
                          "but the label named '%s' was not defined.\n",
                          lab_name);

  *value = convert_uint32(s->address);

  return ASM_NO_ERROR;
}

enum Err_Asm
pass_2_instruction_ops_set(struct Assembler_Processing *asp,
                           const struct Instruction_Statement *is,
                           union op_value (*op_values)[KMA_MAX_OPERANDS]) {
  int i = 0;
  RET_VERBOSE_CLN_IF_FAIL(asp && is && op_values, ASM_INVALID_ARGS,
                          "but something went wrong.");

  RET_VERBOSE_CLN_IF_FAIL(
      cdsg_app_op(asp->cdsg, is->descriptor->opcode), ASM_CDSG_CANNOT_APPEND,
      "but couldn't append OP-code '%02x' to code segment.\n",
      is->descriptor->opcode);

  for (i = 0; i < is->operand_count; i++) {
    if (is->operands[i].type == OP_REG) {
      RET_VERBOSE_CLN_IF_FAIL(
          cdsg_app_reg(asp->cdsg, (*op_values)[i].ui8), ASM_CDSG_CANNOT_APPEND,
          "but couldn't append register code '%02x' to code segment.\n",
          (*op_values)[i].ui8);
    } else {
      RET_VERBOSE_CLN_IF_FAIL(
          cdsg_app_imm(asp->cdsg, (*op_values)[i].i32), ASM_CDSG_CANNOT_APPEND,
          "but couldn't append immediate 32-bit value '%d' to code segment.\n",
          (*op_values[i]).i32);
    }
  }

  return ASM_NO_ERROR;
}

int pass2_register_name_to_value(const char *name, uint8_t *value) {
  RETURN_IF_FAIL(name && value, 0);

  if (strcmp(name, "A") == 0) {
    *value = REG_CODE_A;
  } else if (strcmp(name, "B") == 0) {
    *value = REG_CODE_B;
  } else if (strcmp(name, "C") == 0) {
    *value = REG_CODE_C;
  } else if (strcmp(name, "D") == 0) {
    *value = REG_CODE_D;
  } else if (strcmp(name, "S") == 0) {
    *value = REG_CODE_S;
  } else if (strcmp(name, "SP") == 0) {
    *value = REG_CODE_SP;
  } else {
    return 0;
  }

  return 1;
}
