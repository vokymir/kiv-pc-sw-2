#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "common.h"
#include "memory.h"
#include "symbol.h"

// ===== PRIVATE FUNCTION DEFINITIONS =====

// Grow sym array if needed. Ensures the array have capacity of current +
// additional_symbols*sizeof(symbol). Return 0 on failure, 1 on success.
static int _symtab_ensure_capacity(struct Symbol_Table *symtab,
                                   size_t additional_symbols);

// ===== PUBLIC FUNCTIONS =====

struct Symbol_Table *symtab_create(void) {
  struct Symbol_Table *table = NULL;

  table = jalloc(sizeof(struct Symbol_Table));
  CLEANUP_IF_FAIL(table);

  CLEANUP_IF_FAIL(symtab_init(table));

  return table;

cleanup:
  if (table) {
    jree(table);
  }
  return NULL;
}

int symtab_init(struct Symbol_Table *table) {
  CLEANUP_IF_FAIL(table);

  table->symbols = jalloc(SYMTAB_INITIAL_CAPACITY * sizeof(struct Symbol));
  CLEANUP_IF_FAIL(table->symbols);

  table->count = 0;
  table->capacity = SYMTAB_INITIAL_CAPACITY;

  return 1;

cleanup:
  return 0;
}

void symtab_deinit(struct Symbol_Table *table) {
  CLEANUP_IF_FAIL(table);

  if (table->symbols) {
    jree(table->symbols);
  }

  table->count = 0;
  table->capacity = 0;

cleanup:
  return;
}

void symtab_free(struct Symbol_Table **table) {
  CLEANUP_IF_FAIL(table);

  symtab_deinit(*table);
  jree(*table);
  *table = NULL;

cleanup:
  return;
}

struct Symbol *symtab_add(struct Symbol_Table *table, const char *name,
                          const uint32_t address) {
  struct Symbol *symbol = NULL;
  CLEANUP_IF_FAIL(table && table->symbols && name);
  symbol = &table->symbols[table->count];

  CLEANUP_IF_FAIL(_symtab_ensure_capacity(table, 1));

  symbol->address = address;
  strcpy(symbol->name, name);
  table->count++;

  return symbol;

cleanup:
  return NULL;
}

struct Symbol *symtab_find(const struct Symbol_Table *table, const char *name) {
  size_t i = 0;
  struct Symbol *symbol = NULL;
  CLEANUP_IF_FAIL(table && table->symbols && name);

  for (i = 0; i < table->count; i++) {
    symbol = &table->symbols[i];
    if (strcmp(symbol->name, name) == 0) {
      return symbol; // found
    }
  }

cleanup:
  return NULL; // not found
}

// ===== PRIVATE FUNCTIONS =====

static int _symtab_ensure_capacity(struct Symbol_Table *symtab,
                                   size_t additional_symbols) {
  size_t req = 0, new_cap = 0;
  struct Symbol *new_s = NULL;
  CLEANUP_IF_FAIL(symtab && symtab->symbols);

  if (additional_symbols == 0) {
    return 1;
  }

  req = symtab->count + additional_symbols;
  if (req <= symtab->capacity) {
    return 1;
  }

  new_cap = symtab->capacity ? symtab->capacity : SYMTAB_INITIAL_CAPACITY;
  while (new_cap < req) {
    new_cap *= SYMTAB_CAPACITY_MULT;
  }

  new_s = jealloc(symtab->symbols, new_cap * sizeof(struct Symbol));
  CLEANUP_IF_FAIL(new_s);

  symtab->symbols = new_s;
  symtab->capacity = new_cap;
  return 1;

cleanup:
  return 0;
}
