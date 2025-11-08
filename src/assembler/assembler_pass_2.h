#ifndef ASSEMBLER_PASS_2_H
#define ASSEMBLER_PASS_2_H
/* Module helper for functionality specific to 2nd pass. */

#include <stddef.h>

#include "assembler.h"
#include "parser.h"

// Possible operand values - can be either uint8 or int32.
union op_value {
  uint8_t ui8;
  int32_t i32;
};

// Process one <line> in the second pass of the assembler code.
// Might edit <context> or any structure inside <asp>, based on what is on the
// given <line>. Return corresponding error code.
enum Err_Asm pass2_line(struct Assembler_Processing *asp,
                        enum Assembler_Context *ctx, size_t nl,
                        const char *line);

// Decide which function is called based on information from already parsed line
// saved inside <pstmt>. Pass the error the called function returned.
enum Err_Asm pass2_decide(struct Parsed_Statement *pstmt,
                          struct Assembler_Processing *asp,
                          enum Assembler_Context *ctx, size_t nl);

// Process data declaration in the 2nd pass by actually saving it to
// <asp>->dtsg.
enum Err_Asm pass2_data_decl(struct Parsed_Statement *pstmt,
                             struct Assembler_Processing *asp,
                             enum Assembler_Context *ctx, size_t nl);

// Process instruction in the 2nd pass by actually saving it to <asp>->cdsg.
enum Err_Asm pass2_instruction(struct Parsed_Statement *pstmt,
                               struct Assembler_Processing *asp,
                               enum Assembler_Context *ctx, size_t nl);

// Process data declaration segment of uninitialized value.
enum Err_Asm pass2_data_decl_uninit(struct Assembler_Processing *asp,
                                    const struct Init_Segment *is,
                                    enum Data_Type dt);

// Process data declaration segment of initialized value.
enum Err_Asm pass2_data_decl_value(struct Assembler_Processing *asp,
                                   const struct Init_Segment *is,
                                   enum Data_Type dt);

// Process data declaration segment of string.
enum Err_Asm pass2_data_decl_string(struct Assembler_Processing *asp,
                                    const struct Init_Segment *is);

// Process data declaration segment of DUP statement, i.e. array declaration.
enum Err_Asm pass2_data_decl_dup(struct Assembler_Processing *asp,
                                 const struct Init_Segment *is,
                                 enum Data_Type dt);

// Get operand value from instruction statement. Set values in the array
// op_values. Return adequate ASM error.
enum Err_Asm
pass2_instruction_ops_get(struct Assembler_Processing *asp,
                          const struct Instruction_Statement *is,
                          union op_value (*op_values)[KMA_MAX_OPERANDS]);

// Get/Set one operand on index <idx> from instruction statement <is>. Set
// op_value on success.
enum Err_Asm pass2_intstruction_get_op(struct Assembler_Processing *asp,
                                       const struct Instruction_Statement *is,
                                       union op_value *op_value, int idx);

// Get/Set the value of operand assuming it is a register.
enum Err_Asm pass2_instruction_get_op_reg(struct Assembler_Processing *asp,
                                          const char *reg_name, uint8_t *value);

// Get/Set the value of operand assuming it is a label.
enum Err_Asm pass2_instruction_get_op_label(struct Assembler_Processing *asp,
                                            const char *lab_name,
                                            int32_t *value);

// Append (set) instruction in codesegment based on op_values.
enum Err_Asm
pass_2_instruction_ops_set(struct Assembler_Processing *asp,
                           const struct Instruction_Statement *is,
                           union op_value (*op_values)[KMA_MAX_OPERANDS]);

// Convert register <name> used in assembly to its numeric value in binary.
// On success set value to corresponding number & return 1.
// On failure only return 0.
int pass2_register_name_to_value(const char *name, uint8_t *value);

#endif
