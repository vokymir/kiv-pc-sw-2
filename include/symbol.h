#ifndef SYMBOL_H
#define SYMBOL_H
/* Module for the symbol table which is used to store position of variables and
 * labels in data and code segments respectively. */

#include <stddef.h>
#include <stdint.h>

#include "common.h"

// One record containing variable/label name and its address in corresponding
// data/code segment.
struct Symbol {
  char name[SYMTAB_MAX_NAME_LEN];
  uint32_t address;
};

// Table containing every stored symbol. Can be used in 1st pass to store the
// positions and in 2nd pass to retrieve the positions.
// The abbreviation SYMTAB for SYMbol TABle is used.
struct Symbol_Table {
  struct Symbol *symbols;
  size_t count; // how many symbols are in the table
  size_t capacity;
};

// Create a symbol table and initialize it by calling symtab_init. Return
// pointer to Symbol table on success, NULL on fail.
// Caller must free using symtab_free
struct Symbol_Table *symtab_create(void);

// Initializes the insides of symbol table.
int symtab_init(struct Symbol_Table *table);

// Free all insides of symbol table.
void symtab_deinit(struct Symbol_Table *table);

// Call deinit to free all insides, then frees the symtab itself.
// Set the pointer to table to NULL.
void symtab_free(struct Symbol_Table **table);

// Add a new symbol inside a <table>. Will COPY the <name>, so caller retains
// ownership of the original! Return pointer to newly created symbol on success,
// NULL on failure.
struct Symbol *symtab_add(struct Symbol_Table *table, const char *name,
                          const uint32_t address);

// Find a symbol by its name (mnemonic) in a <table>.
// Return pointer to symbol or NULL on failure.
struct Symbol *symtab_find(const struct Symbol_Table *table, const char *name);

#endif
