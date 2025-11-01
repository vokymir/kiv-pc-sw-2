#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "codeseg.h"
#include "common.h"
#include "dataseg.h"
#include "symbol.h"

#define KMA_CDSG_BYTES (256 * 1024)
#define KMA_DTSG_BYTES (256 * 1024)

struct Assembler_Processing {
  const struct Config *config;
  struct Symbol_Table *symtab;
  struct Data_Segment *dtsg;
  struct Code_Segment *cdsg;
};

enum Assembler_Context {
  ASC_FILE_START,
  ASC_AFTER_KMA,
  ASC_DATA,
  ASC_CODE,
};

enum Err_Asm {
  ASM_NO_ERROR,
  ASM_KMA_EXPECTED,
  ASM_KMA_DOUBLE,
  ASM_CANNOT_OPEN_FILE,
  ASM_INVALID_ARGS,
  ASM_CREATING_TOKENS,
  ASM_CREATING_PSTMT,
  ASM_DATA_ABROAD,
  ASM_CODE_ABROAD,
  ASM_UNKNOWN_PSTMT_TYPE,
  ASM_DTSG_CANNOT_ADVANCE,
  ASM_CDSG_CANNOT_ADVANCE,
  ASM_SYMTAB_CANNOT_ADD,
  ASM_SYMTAB_ALREADY_EXIST,
  ASM_INVALID_INSTUCTION,
  ASM_DTSG_TOO_LARGE,
  ASM_CDSG_TOO_LARGE,
};

// Wrapper around 2-pass assembler to binary process.
// Return exact error code.
enum Err_Main process_assembler(struct Assembler_Processing *asp);

// First pass of assembler code = evaluates the whole file, creates a symbol
// table and fills it with actual values in code/data segment. Return error
// codes based on assignment error codes table
enum Err_Asm pass1(struct Assembler_Processing *asp);

// Second pass of assembler code = evaluates the whole file, using a symbol
// table it writes into code segment with actual values. Return adequate error
// code.
enum Err_Asm pass2(struct Assembler_Processing *asp);

// Create new ASsembler Processing struct. Call asp_init to initialize from
// given parameters. If any is missing (NULL), the init will allocate new.
// Only exception is config, which can only be given.
struct Assembler_Processing *asp_create(const struct Config *config,
                                        struct Symbol_Table *symtab,
                                        struct Data_Segment *dtsg,
                                        struct Code_Segment *cdsg);

// Initialize any given ASsembler Processing struct with given paramters. If any
// is NULL, allocates new.
// Return 1 on success, 0 on failure.
int asp_init(struct Assembler_Processing *asp, const struct Config *config,
             struct Symbol_Table *symtab, struct Data_Segment *dtsg,
             struct Code_Segment *cdsg);

// Free all inside structures of given asp.
void asp_deinit(struct Assembler_Processing *asp);

// Call asp_deinit & free ASP, then set pointer to NULL.
void asp_free(struct Assembler_Processing **asp);

#endif
