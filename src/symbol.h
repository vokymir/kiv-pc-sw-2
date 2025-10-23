#ifndef SYMBOL_H
#define SYMBOL_H

#include <stddef.h>
#include <stdint.h>

#define SYMTAB_INITIAL_CAPACITY 16
#define SYMTAB_CAPACITY_MULT 2
#define SYMTAB_MAX_NAME_LEN 256

struct Symbol {
  char name[SYMTAB_MAX_NAME_LEN];
  uint32_t address;
};

struct Symbol_Table {
  struct Symbol *symbols;
  size_t count;
  size_t capacity;
};

// Create a symbol table and initialize it by calling symtab_init. Return
// pointer on success, NULL on fail.
struct Symbol_Table *symtab_create(void);

// Initializes the insides of symbol table.
int symtab_init(struct Symbol_Table *table);

// Free all insides of symbol table.
void symtab_deinit(struct Symbol_Table *table);

// Call deinit to free all insides, then frees the symtab itself.
// Set the pointer to it to NULL.
void symtab_free(struct Symbol_Table **table);

// Add a new symbol inside a table. Will COPY the name, so caller retains
// ownership of the original! Return pointer to newly created symbol on success,
// NULL on failure.
struct Symbol *symtab_add(struct Symbol_Table *table, const char *name,
                          const uint32_t address);

// Find a symbol by its name (mnemonic) in a table.
// Return pointer to symbol or NULL on failure.
struct Symbol *symtab_find(const struct Symbol_Table *table, const char *name);

#endif
