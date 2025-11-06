#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include "codeseg.h"
#include "common.h"
#include "dataseg.h"
#include "symbol.h"

// Stores intermediate information about assembler code during the processing of
// the file.
struct Assembler_Processing {
  const struct Config *config; // For source/target path and -v -i flags
  struct Symbol_Table *symtab; // Store positions of labels and data identifiers
  struct Data_Segment *dtsg;   // Store the data segment bytes = declarations
  struct Code_Segment *cdsg;   // Store the code segment = instructions
};

// The assembler file have 4 states:
enum Assembler_Context {
  ASC_FILE_START, // Beginning of file - nothing (except for comments or empty
                  // lines) detected
  ASC_AFTER_KMA, // The line with .KMA confirming the file type was found. Now a
                 // .DATA or .CODE must be next (or comments or empty lines)
  ASC_DATA,      // Section with data declarations
  ASC_CODE,      // Section with instructions
};

// Error codes specific for Assembler module. All are mapped to Err_Main and can
// be returned as such.
enum Err_Asm {
  ASM_NO_ERROR,     // Everything is OK
  ASM_KMA_EXPECTED, // Found something before the initiating .KMA at the start
                    // of file
  ASM_KMA_DOUBLE,   // The .KMA was found twice, which is illegal
  ASM_CANNOT_OPEN_FILE, // Cannot open source file
  ASM_INVALID_ARGS,     // Arguments given to any function weren't valid
  ASM_CREATING_TOKENS,  // Couldn't tokenize some line using lexer
  ASM_CREATING_PSTMT,   // Couldn't parse tokens from some line using parser
  ASM_DATA_ABROAD, // Data declaration was found on place where it shouldn't be
  ASM_CODE_ABROAD, // Instruction/Label definition found outside .CODE block
  ASM_UNKNOWN_PSTMT_TYPE,  // When parsing a line UNKNOWN token was detected and
                           // that resulted in unknown parsed statement type
  ASM_DTSG_CANNOT_ADVANCE, // During 1st pass cannot advance the pointer in
                           // datasegment
  ASM_CDSG_CANNOT_ADVANCE, // During 1st pass cannot advance the pointer in
                           // codesegment
  ASM_SYMTAB_CANNOT_ADD,   // During 1st pass cannot add label/variable to the
                           // symbol table
  ASM_SYMTAB_ALREADY_EXIST, // During 1st pass was some identifier definition
                            // (label/variable) found twice which is illegal
  ASM_INVALID_INSTUCTION,   // During 1st pass a found instruction was unknown
  ASM_DTSG_TOO_LARGE,   // The assembler program tried to allocate more data in
                        // the datasegment than the KMA computer allows
  ASM_CDSG_TOO_LARGE,   // The assembler program has too many instructions that
                        // they cannot fit the code segment
  ASM_UNKNOWN_INIT_SEG, // During 2nd pass in data declaration was found
                        // something illegal
  ASM_DTSG_CANNOT_APPEND, // During 2nd pass cannot append bytes to the
                          // data segment
  ASM_CDSG_CANNOT_APPEND, // During 2nd pass cannot apend bytes to the code
                          // segment
  ASM_INVALID_OPERAND_REGISTER, // During 2nd pass was found register name which
                                // the KMA computer does not have (or provide
                                // programatic access to)
  ASM_INVALID_OPERAND_LABEL,    // During 2nd pass was found jump to label which
                                // wasn't defined
};

// Process the assembler .kma file (on path specified in asp) using a Two pass
// method. Return error codes as defined in assignment/common.h
enum Err_Main process_assembler(struct Assembler_Processing *asp);

// First pass of assembler code = evaluate the file, creates a symbol table and
// fills it with positions of symbols. Return ASM error codes defined in
// assembler.h
enum Err_Asm pass1(struct Assembler_Processing *asp);

// Second pass of assembler code = evaluate the file, write into code and data
// segment with absolute values using a symbol table from asp. Return ASM error
// code defined in assembler.h.
// WARNING: Does not check for syntax which isn't
// directly needed. The assembler file is already validated in 1st pass.
enum Err_Asm pass2(struct Assembler_Processing *asp);

// Create new Assembler Processing struct. Call asp_init to initialize based on
// given arguments. Return pointer to the struct or NULL on failure.
struct Assembler_Processing *asp_create(const struct Config *config,
                                        struct Symbol_Table *symtab,
                                        struct Data_Segment *dtsg,
                                        struct Code_Segment *cdsg);

// Initialize any given Assembler Processing struct with given arguments. If any
// argument is NULL, allocate new, except for config (that is needed for
// stderr/verbose output and therefore is required) which if missing caused the
// function to fail. Return 1 on success, 0 on failure.
int asp_init(struct Assembler_Processing *asp, const struct Config *config,
             struct Symbol_Table *symtab, struct Data_Segment *dtsg,
             struct Code_Segment *cdsg);

// Free all inside structures of given Assembler Processing struct and set the
// pointers to NULL.
void asp_deinit(struct Assembler_Processing *asp);

// Call asp_deinit & free ASP, then set pointer to ASP to NULL.
void asp_free(struct Assembler_Processing **asp);

#endif
