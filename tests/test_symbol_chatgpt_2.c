#include "../src/memory.h"
#include "../src/symbol.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST(name) printf("\n=== %s ===\n", name)

int main(void) {
  printf("Running Symbol table tests (new interface)...\n");

  // === Test 1: Creation ===
  TEST("symtab_create()");
  struct Symbol_Table *table = symtab_create();
  assert(table != NULL);
  assert(table->symbols != NULL);
  assert(table->count == 0);
  assert(table->capacity >= SYMTAB_INITIAL_CAPACITY);
  printf("✅ Created and initialized symbol table.\n");

  // === Test 2: Add symbols ===
  TEST("symtab_add()");
  struct Symbol *s1 = symtab_add(table, "LOOP", 0x1000);
  struct Symbol *s2 = symtab_add(table, "START", 0x0000);
  struct Symbol *s3 = symtab_add(table, "END", 0x1FFF);

  assert(s1 != NULL && s2 != NULL && s3 != NULL);
  assert(strcmp(s1->name, "LOOP") == 0);
  assert(s1->address == 0x1000);
  assert(strcmp(s2->name, "START") == 0);
  assert(s3->address == 0x1FFF);
  assert(table->count == 3);
  printf("✅ Added 3 valid symbols.\n");

  // === Test 3: Duplicate name handling ===
  TEST("symtab_add() duplicate");
  struct Symbol *dup = symtab_add(table, "START", 0x9999);
  assert(dup != NULL); // Allowed unless interface defines otherwise
  assert(strcmp(dup->name, "START") == 0);
  printf("✅ Duplicate allowed (if not explicitly prevented).\n");

  // === Test 4: Invalid inputs ===
  TEST("symtab_add() invalid inputs");
  assert(symtab_add(NULL, "X", 1) == NULL);
  assert(symtab_add(table, NULL, 1) == NULL);
  printf("✅ Invalid input handling works.\n");

  // === Test 5: Find ===
  TEST("symtab_find()");
  struct Symbol *found = symtab_find(table, "LOOP");
  assert(found != NULL);
  assert(found->address == 0x1000);

  found = symtab_find(table, "END");
  assert(found != NULL);
  assert(found->address == 0x1FFF);

  found = symtab_find(table, "MISSING");
  assert(found == NULL);
  printf("✅ Lookup tests passed.\n");

  // === Test 6: Cleanup ===
  TEST("symtab_free()");
  symtab_free(&table);
  assert(table == NULL);
  assert(jemory() == 0);
  printf("✅ Table freed, pointer cleared, no leaks detected.\n");

  printf("\n🎉 All Symbol table tests passed successfully!\n");
  return 0;
}
