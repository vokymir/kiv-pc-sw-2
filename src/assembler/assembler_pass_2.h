#ifndef ASSEMBLER_PASS_2_H
#define ASSEMBLER_PASS_2_H

#include <stddef.h>

#include "assembler.h"
#include "parser.h"

// Possible operand values - can be either uint8 or int32.
union op_value {
  uint8_t ui8;
  int32_t i32;
};

enum Err_Asm pass2_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line);

enum Err_Asm pass2_decide(struct Parsed_Statement *pstmt,
                          struct Assembler_Processing *asp,
                          enum Assembler_Context *ctx, size_t nl);

enum Err_Asm pass2_data_decl(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl);

enum Err_Asm pass2_instruction(struct Parsed_Statement *pstmt,
                               struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl);

enum Err_Asm pass2_data_decl_uninit(struct Assembler_Processing *asp,
                                    const struct Init_Segment *is);

enum Err_Asm pass2_data_decl_value(struct Assembler_Processing *asp,
                                   const struct Init_Segment *is,
                                   enum Data_Type dt);

enum Err_Asm pass2_data_decl_string(struct Assembler_Processing *asp,
                                    const struct Init_Segment *is);

enum Err_Asm pass2_data_decl_dup(struct Assembler_Processing *asp,
                                 const struct Init_Segment *is,
                                 enum Data_Type dt);

// Get operand valued from instruction statement. Set enum array op_values.
// Return adequate ASM error.
enum Err_Asm
pass2_instruction_ops_get(struct Assembler_Processing *asp,
                          const struct Instruction_Statement *is,
                          union op_value (*op_values)[KMA_MAX_OPERANDS]);

// Get operand from instruction statement, on index. Set enum op_value on
// success.
enum Err_Asm pass2_intstruction_get_op(struct Assembler_Processing *asp,
                                       const struct Instruction_Statement *is,
                                       union op_value *op_value, int idx);

// Get the value when operand is Register.
enum Err_Asm pass2_instruction_get_op_reg(struct Assembler_Processing *asp,
                                          const char *reg_name, uint8_t *value);

enum Err_Asm pass2_instruction_get_op_label(struct Assembler_Processing *asp,
                                            const char *lab_name,
                                            int32_t *value);

// Append (set) instruction in codesegment based on op_values.
enum Err_Asm
pass_2_instruction_ops_set(struct Assembler_Processing *asp,
                           const struct Instruction_Statement *is,
                           union op_value (*op_values)[KMA_MAX_OPERANDS]);

// On success change value adequately & return 1.
// On failure only return 0.
int pass2_register_name_to_value(const char *name, uint8_t *value);

#endif
