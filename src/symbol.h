#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdint.h>

struct Symbol {
  char *name;
  uint32_t address;
};

struct Symbol_Table;

// Create a symbol table. Return pointer on success, NULL on fail.
struct Symbol_Table *symtab_create(void);

// Free a symbol table with all its insides.
void symtab_free(struct Symbol_Table *table);

// Add a new symbol inside a table. Will COPY the name, so caller retains
// ownership! Return pointer to newly created symbol on success, NULL on
// failure.
struct Symbol *symtab_add(struct Symbol_Table *table, const char *name,
                          const uint32_t address);

// Find a symbol by its name (mnemonic) in a table.
// Return pointer to symbol or NULL on failure.
struct Symbol *symtab_find(const struct Symbol_Table *table, const char *name);

#endif
