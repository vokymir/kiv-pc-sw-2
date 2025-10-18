#include "../src/memory.h"
#include "../src/symbol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST(name) printf("\n=== %s ===\n", name)

int main(void) {
  printf("Running Symbol table tests...\n");

  // === Test 1: Creation and cleanup ===
  TEST("symtab_create() and symtab_free()");
  struct Symbol_Table *table = symtab_create();
  assert(table != NULL);
  printf("✅ Symbol table created.\n");

  symtab_free(table);
  assert(jemory() == 0);
  printf("✅ Symbol table freed, no memory leaks.\n");

  // === Test 2: Adding symbols ===
  TEST("symtab_add()");
  table = symtab_create();
  assert(table != NULL);

  struct Symbol *symA = symtab_add(table, "START", 0x0040);
  struct Symbol *symB = symtab_add(table, "LOOP", 0x0080);
  struct Symbol *symC = symtab_add(table, "END", 0x0100);

  assert(symA && symB && symC);
  assert(strcmp(symA->name, "START") == 0);
  assert(symB->address == 0x0080);
  printf("✅ Added three symbols.\n");

  // === Test 3: Find symbols ===
  TEST("symtab_find()");
  struct Symbol *foundA = symtab_find(table, "START");
  struct Symbol *foundB = symtab_find(table, "END");
  struct Symbol *foundX = symtab_find(table, "UNKNOWN");

  assert(foundA && strcmp(foundA->name, "START") == 0);
  assert(foundB && foundB->address == 0x0100);
  assert(foundX == NULL);
  printf("✅ Symbol lookup works correctly.\n");

  // === Test 4: Add invalid ===
  TEST("symtab_add() invalid arguments");
  assert(symtab_add(NULL, "X", 1) == NULL);
  assert(symtab_add(table, NULL, 1) == NULL);
  printf("✅ Invalid add tests passed.\n");

  // === Test 5: Memory cleanup ===
  TEST("symtab_free()");
  symtab_free(table);
  assert(jemory() == 0);
  printf("✅ Symbol table freed with no leaks.\n");

  printf("\n🎉 All Symbol tests passed successfully!\n");
  return 0;
}
