#include <stddef.h>
#include <stdint.h>

#include "assembler.h"
#include "codeseg.h"
#include "common.h"
#include "dataseg.h"
#include "memory.h"
#include "symbol.h"

#include "assembler_convert.h"
#include "assembler_passes.h"

enum Err_Main process_assembler(struct Assembler_Processing *asp) {
  enum Err_Asm res = ASM_NO_ERROR;
  if ((res = pass1(asp)) != ASM_NO_ERROR) {
    return convert_err(res);
  }
  cdsg_begin(asp->cdsg); // reuse segments
  dtsg_begin(asp->dtsg); // goto start
  if ((res = pass2(asp)) != ASM_NO_ERROR) {
    return convert_err(res);
  }

  return ERR_NO_ERROR;
}

enum Err_Asm pass1(struct Assembler_Processing *asp) {
  return passes_any_pass(asp, 0);
}

enum Err_Asm pass2(struct Assembler_Processing *asp) {
  return passes_any_pass(asp, 1);
}

struct Assembler_Processing *asp_create(const struct Config *config,
                                        struct Symbol_Table *symtab,
                                        struct Data_Segment *dtsg,
                                        struct Code_Segment *cdsg) {
  struct Assembler_Processing *asp =
      jalloc(sizeof(struct Assembler_Processing));
  RET_STDERR_IF_FAIL(
      asp, NULL, "Couldn't allocate space for Assembler_Processing struct.");

  if (!asp_init(asp, config, symtab, dtsg, cdsg)) {
    asp_free(&asp);
    return NULL;
  }
  return asp;
}

int asp_init(struct Assembler_Processing *asp, const struct Config *config,
             struct Symbol_Table *symtab, struct Data_Segment *dtsg,
             struct Code_Segment *cdsg) {
  RET_STDERR_IF_FAIL(asp && config, 0, "Invalid pointer in arguments.");
  asp->config = config;

  if (symtab) {
    asp->symtab = symtab;
  } else {
    asp->symtab = symtab_create();
  }
  CLEANUP_IF_FAIL_ERR(
      asp->symtab,
      "Couldn't create symbol table when initializing assembler processing.");

  if (dtsg) {
    asp->dtsg = dtsg;
  } else {
    asp->dtsg = dtsg_create();
  }
  CLEANUP_IF_FAIL_ERR(
      asp->dtsg,
      "Couldn't create data segment when initializing assembler processing.");

  if (cdsg) {
    asp->cdsg = cdsg;
  } else {
    asp->cdsg = cdsg_create();
  }
  CLEANUP_IF_FAIL_ERR(
      asp->cdsg,
      "Couldn't create code segment when initializing assembler processing.");

  return 1;

cleanup:
  asp_deinit(asp);
  return 0;
}

void asp_deinit(struct Assembler_Processing *asp) {
  if (!asp) {
    PRINT_ERR("Tried to deinit an assembler processing struct, but gave NULL "
              "pointer.");
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
    PRINT_ERR(
        "Tried to free an assembler processing struct, but it doesn't exist.");
    return;
  }
  asp_deinit(*asp);
  jree(*asp);
  *asp = NULL;
}
