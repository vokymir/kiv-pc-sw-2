#include <stdint.h>

#include "assembler.h"
#include "common.h"
#include "memory.h"
#include "parser.h"

#include "assembler_convert.h"

int32_t convert_uint32(uint32_t u) {
  return u > INT32_MAX ? INT32_MAX : (int32_t)u;
}

int32_t convert_size_t(size_t s) {
  return s > INT32_MAX ? INT32_MAX : (int32_t)s;
}

enum Err_Main convert_err(enum Err_Asm err) {
  switch (err) {
  case ASM_NO_ERROR:
    return ERR_NO_ERROR;
  case ASM_KMA_EXPECTED:
  case ASM_KMA_DOUBLE:
  case ASM_DATA_ABROAD:
  case ASM_CODE_ABROAD:
  case ASM_SYMTAB_ALREADY_EXIST:
  case ASM_INVALID_INSTUCTION:
  case ASM_INVALID_OPERAND_REGISTER:
  case ASM_INVALID_OPERAND_LABEL:
  case ASM_INVALID_OPERAND_OFFSET:
    return ERR_SYNTAX_ERROR;
  case ASM_INVALID_ARGS:
  case ASM_CREATING_TOKENS:
  case ASM_CREATING_PSTMT:
  case ASM_UNKNOWN_PSTMT_TYPE:
  case ASM_SYMTAB_CANNOT_ADD:
  case ASM_UNKNOWN_INIT_SEG:
    return ERR_MY_CODE_FAILURE;
  case ASM_DTSG_CANNOT_ADVANCE:
  case ASM_CDSG_CANNOT_ADVANCE:
  case ASM_DTSG_TOO_LARGE:
  case ASM_CDSG_TOO_LARGE:
  case ASM_DTSG_CANNOT_APPEND:
  case ASM_CDSG_CANNOT_APPEND:
    return ERR_OUT_OF_MEMORY;
  case ASM_CANNOT_OPEN_FILE:
    return ERR_FILE_ACCESS_FAILURE;
  default:
    PRINT_ERR("Unknown assembler error code detected.");
    return ERR_MY_CODE_FAILURE;
  }
}

const struct Token **convert_tokens_to_arr(const struct Token *orig) {
  size_t count = 0, i = 0;
  const struct Token **res = NULL;
  RET_STDERR_IF_FAIL(
      orig, NULL, "Tried to convert Token array, but NULL pointer was given.");

  while (orig[count].type != TOKEN_EOF) {
    count++;
  }
  count++; // also count EOF

  res = jalloc((count + 1) *
               sizeof(*res)); // +1 for NULL terminator (for better sleep)
  RET_STDERR_IF_FAIL(res, NULL,
                     "Couldn't allocate space for array of pointers to Tokens "
                     "(when converting).");

  for (i = 0; i < count; i++) {
    res[i] = &orig[i];
  }
  res[count] = NULL;

  return res;
}

void convert_free_tokens_arr(const struct Token **tokens[]) {
  if (!tokens || !*tokens) {
    PRINT_ERR(
        "Tried to free converted token array, but the pointer given was NULL.");
    return;
  }
  jree(*tokens);
  *tokens = NULL;
}

struct Parsed_Statement *convert_parse_tokens(const struct Token *tokens,
                                              size_t nl) {
  struct Parsed_Statement *pstmt = NULL;
  const struct Token **converted_tokens = NULL;
  RET_STDERR_IF_FAIL(
      tokens, 0,
      "Tried to parse tokens, but the pointer to 'tokens' was NULL.");

  converted_tokens = convert_tokens_to_arr(tokens);
  RET_STDERR_IF_FAIL(converted_tokens, NULL,
                     "Couldn't covert tokens to array.");

  pstmt = parse_tokens(converted_tokens, nl);
  convert_free_tokens_arr(&converted_tokens);

  return pstmt;
}
