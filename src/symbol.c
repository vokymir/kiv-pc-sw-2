#include <stdint.h>
#include <string.h>

#include "common.h"
#include "llist.h"
#include "memory.h"
#include "symbol.h"

// ===== STRUCT DEFINITIONS =====

struct Symbol_Table {
  struct Llist *symbols;
};

// ===== PRIVATE FUNCTION DEFINITIONS =====

// Create new symbol. COPY the name to own new location.
// If some arg is not provided, the symbol will be created with uninicialized
// parameter, but would create.
// Return NULL on failure, pointer on success.
static struct Symbol *_symtab_create_symbol(const char *name, uint32_t addr);

// Free symbol and its insides.
static void _symtab_free_symbol(struct Symbol *symbol);

// ===== PUBLIC FUNCTIONS =====

struct Symbol_Table *symtab_create(void) {
  struct Symbol_Table *table = NULL;

  table = jalloc(sizeof(struct Symbol_Table));
  CLEANUP_IF_FAIL(table);

  table->symbols = llist_create(sizeof(struct Symbol));
  CLEANUP_IF_FAIL(table->symbols);

  return table;

cleanup:
  if (table) {
    jree(table);
  }
  return NULL;
}

void symtab_free(struct Symbol_Table *table) {
  if (!table) {
    return;
  }
  if (table->symbols) {
    llist_free(table->symbols, (llist_free_node_data)_symtab_free_symbol);
  }
  jree(table);
  return;
}

struct Symbol *symtab_add(struct Symbol_Table *table, const char *name,
                          const uint32_t address) {
  struct Symbol *symbol = NULL;
  void *tmp = NULL;
  if (!table || !table->symbols || !name) {
    return NULL;
  }

  symbol = _symtab_create_symbol(name, address);
  CLEANUP_IF_FAIL(symbol);

  tmp = llist_add(table->symbols, (void **)&symbol);
  CLEANUP_IF_FAIL(tmp);
  symbol = tmp;

  return symbol;

cleanup:
  if (symbol) {
    _symtab_free_symbol(symbol);
  }
  return NULL;
}

struct Symbol *symtab_find(const struct Symbol_Table *table, const char *name) {
  size_t i = 0;
  struct Symbol *symbol = NULL;
  if (!table || !table->symbols || !name) {
    return NULL;
  }

  for (i = 0; i < table->symbols->count; i++) { // iterate all symbols
    symbol = llist_get(table->symbols, i);
    if (symbol && symbol->name && strcmp(symbol->name, name) == 0) {
      return symbol; // found with the same name
    }
    symbol = NULL;
  }

  return NULL; // not found
}

// ===== PRIVATE FUNCTIONS =====

static struct Symbol *_symtab_create_symbol(const char *name, uint32_t addr) {
  struct Symbol *symbol = NULL;

  symbol = jalloc(sizeof(struct Symbol));
  CLEANUP_IF_FAIL(symbol);

  if (name) {
    symbol->name = jtrdup(name);
  }
  CLEANUP_IF_FAIL(symbol->name);

  symbol->address = addr;

  return symbol;

cleanup:
  if (symbol) {
    if (symbol->name) {
      jree(symbol->name);
    }
    jree(symbol);
  }
  return NULL;
}

static void _symtab_free_symbol(struct Symbol *symbol) {
  if (!symbol) {
    return;
  }
  if (symbol->name) {
    jree(symbol->name);
  }
  jree(symbol);
  return;
}
