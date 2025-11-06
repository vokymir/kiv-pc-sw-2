#ifndef COMMON_H
#define COMMON_H

#include <stddef.h>
#include <stdlib.h>

// Is in parser, which cannot be included (recursive includes).
struct Instruction_Statement;

// =========================
// ===== MACRO HELPERS =====
// =========================

// Implementing RAII-like cleanup procedure.
// If condition is not satisfied, go to label.
// Label must be at the end of the same function, to work reliably.
// Can optionally print error message, wrappers defined below.
#define GOTO_IF_FAIL_OPTS(cond, label, print_err, ...)                         \
  do {                                                                         \
    if (!(cond)) {                                                             \
      if ((print_err)) {                                                       \
        PRINT_ERR(__VA_ARGS__);                                                \
      }                                                                        \
      goto label;                                                              \
    }                                                                          \
  } while (0)

#define GOTO_IF_FAIL(cond, label) GOTO_IF_FAIL_OPTS((cond), (label), 0, "")

#define GOTO_IF_FAIL_ERR(cond, label, ...)                                     \
  GOTO_IF_FAIL_OPTS((cond), (label), 1, __VA_ARGS__)

// If condition is not satisfied, go to label 'cleanup'.
#define CLEANUP_IF_FAIL(cond) GOTO_IF_FAIL((cond), cleanup)

#define CLEANUP_IF_FAIL_ERR(cond, ...)                                         \
  GOTO_IF_FAIL_ERR((cond), cleanup, __VA_ARGS__)

#define RETURN_IF_FAIL(cond, retval)                                           \
  do {                                                                         \
    if (!(cond)) {                                                             \
      return retval;                                                           \
    }                                                                          \
  } while (0)

// Return Err & print error message to stderr
#define RET_STDERR(err, ...)                                                   \
  do {                                                                         \
    PRINT_ERR(__VA_ARGS__);                                                    \
    return err;                                                                \
  } while (0)

// If condition is not satisfied, return err & print ... to stderr.
#define RET_STDERR_IF_FAIL(cond, err, ...)                                     \
  do {                                                                         \
    if (!(cond)) {                                                             \
      RET_STDERR(err, __VA_ARGS__);                                            \
    }                                                                          \
  } while (0)

// =======================
// ===== ERROR CODES =====
// =======================

// Errors specific to main, which the program outputs.
enum Err_Main {
  ERR_NO_ERROR = 0,
  ERR_INVALID_INPUT_FILE = 1,
  ERR_INVALID_OUTPUT_FILE = 2,
  ERR_SYNTAX_ERROR = 3,
  ERR_FILE_ACCESS_FAILURE = 4,
  ERR_OUT_OF_MEMORY = 5,
  ERR_UNRESOLVED_REFERENCE = 6,
  ERR_CODE_SEGMENT_TOO_LARGE = 7,
  ERR_DATA_SEGMENT_TOO_LARGE = 8,
  ERR_MY_CODE_FAILURE = 42,
};

// ==================
// ===== CONFIG =====
// ==================

// Holds information needed throughout the whole program.
struct Config {
  int flag_verbose;
  int flag_instruction;
  char *source;
  char *target;
};

// ==================
// ===== PRINTS =====
// ==================

// United verbose output to console.
// Only print if condition is met, variadic arguments will be forwarded to
// printf function from stdio.h, which require the first argument to be
// *string*, the next anything 'mentioned' in the string.
void print_verbose(int condition, const char *string, ...)
    __attribute__((format(printf, 2, 3)));

// Don't print the [VERBOSE] in the beginning.
// Only print if condition is met, variadic arguments will be forwarded to
// printf function from stdio.h, which require the first argument to be
// *string*, the next anything 'mentioned' in the string.
void print_verbose_clean(int condition, const char *string, ...)
    __attribute__((format(printf, 2, 3)));

// United instruction output to console.
// Only print if condition is met.
// Format:
// L<line>: <is.name> [<is.operands>] at CS:<addr>
// e.g.:
// L50: DEC A at CS:123
void print_instruction(int condition, size_t line,
                       const struct Instruction_Statement *is, size_t addr);

// Print error message to stderr.
void print_err(const char *filename, size_t line, const char *string, ...)
    __attribute__((format(printf, 3, 4)));
// Use this macro to get correct filename & line number to the print_err
// function above.
#define PRINT_ERR(...) print_err(__FILE__, __LINE__, __VA_ARGS__)

// ===========================
// ===== VARIOUS HELPERS =====
// ===========================

// Return the index of first argument which is NULL.
// The indicies are 1 indexed !!
// Return 0 if none is NULL.
size_t first_null_arg(const void *args[], size_t count);

// Return the 1-based index of first NULL argument, or 0 if no argument is NULL.
#define FIRST_NULL(...)                                                        \
  first_null_arg((const void *[]){__VA_ARGS__},                                \
                 sizeof((const void *[]){__VA_ARGS__}) / sizeof(const void *))

// Print to stderr which argument was invalid (first).
#define PRINT_ERR_1ST_NULL_ARG(...)                                            \
  PRINT_ERR("Invalid argument at %zu. position.", FIRST_NULL(__VA_ARGS__))

// Return 1 if all arguments are valid => are not NULL.
int valid_args(size_t count, ...);

// Return 1 if all arguments are not NULL
#define VALID_ARGS(...)                                                        \
  valid_args(sizeof((const void *[]){__VA_ARGS__}) / sizeof(const void *),     \
             __VA_ARGS__)

// =====================================
// ===== KM MACHINE SPECIFICATIONS =====
// =====================================

// Memory segment sizes
#define KMA_CDSG_BYTES (256 * 1024) // 256 kB
#define KMA_DTSG_BYTES (256 * 1024) // 256 kB
#define KMA_STSG_BYTES (16 * 1024)  // 16 kB

// KMX binary
#define KMX_SIGNATURE_LEN 3
#define KMX_SIGNATURE "KMX" // File format signature

// instruction limits
#define KMA_MAX_OPERANDS 2 // Max operands per instruction

// Register values (specified in assignment)
enum Reg_Code {
  REG_CODE_A = 0x01,
  REG_CODE_B = 0x02,
  REG_CODE_C = 0x03,
  REG_CODE_D = 0x04,
  REG_CODE_S = 0x05,
  REG_CODE_SP = 0x06,
};

// Data type sizes
#define KMA_DWORD_SIZE 4 // DWORD is 4 bytes (32-bit)
#define KMA_BYTE_SIZE 1  // BYTE is 1 byte (8-bit)

// ==========================
// ===== LEXER & PARSER =====
// ==========================

#define MAX_IDENTIFIER_LEN 256 // Variable/label names in assembly
#define MAX_LABEL_NAME_LEN                                                     \
  MAX_IDENTIFIER_LEN // Jump target labels with @ prefix
#define SYMTAB_MAX_NAME_LEN                                                    \
  MAX_IDENTIFIER_LEN // Identifier = variable/@label in symbol table

#define MAX_REGISTER_NAME_LEN                                                  \
  3 // Register names like "SP", add 1 for \0 just for certainty
#define TOKEN_MAX_VALUE_LEN MAX_IDENTIFIER_LEN // Token string content
#define MAX_INIT_SEGMENT_STRING_LEN                                            \
  TOKEN_MAX_VALUE_LEN // String literals in .DATA, must be before processed in
                      // tokens so make sense

#define MAX_ERROR_MSG_LEN 512

// ==========================
// ===== DYNAMIC ARRAYS =====
// ==========================

#define DYNAMIC_ARRAY_INITIAL_CAPACITY 16
#define DYNAMIC_ARRAY_GROWTH_MULTIPLIER 2

// === SPECIFIC ARRAYS ===
#define CDSG_INITIAL_CAPACITY DYNAMIC_ARRAY_INITIAL_CAPACITY
#define CDSG_CAPACITY_MULT DYNAMIC_ARRAY_GROWTH_MULTIPLIER

#define DTSG_INITIAL_CAPACITY DYNAMIC_ARRAY_INITIAL_CAPACITY
#define DTSG_CAPACITY_MULT DYNAMIC_ARRAY_GROWTH_MULTIPLIER

#define SYMTAB_INITIAL_CAPACITY DYNAMIC_ARRAY_INITIAL_CAPACITY
#define SYMTAB_CAPACITY_MULT DYNAMIC_ARRAY_GROWTH_MULTIPLIER

#define TOKENS_INITIAL_CAPACITY DYNAMIC_ARRAY_INITIAL_CAPACITY
#define TOKENS_CAPACITY_MULT DYNAMIC_ARRAY_GROWTH_MULTIPLIER

#define FU_GETLINE_INIT_LEN 128
#define FU_GETLINE_CAP_MULT DYNAMIC_ARRAY_GROWTH_MULTIPLIER

#endif
