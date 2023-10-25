#include "../../symtable.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int failures = 0;

#define TEST(cond)                                    \
    if (!(cond))                                      \
    {                                                 \
        printf("FAIL[ln %d]\t%s\n", __LINE__, #cond); \
        failures++;                                   \
    }

SymTab_T *table = NULL;

char** keys = {"global", "local", "daco143", "kluc", "$XYZ"};

void test() {
    if (!SymTabInit(table)) return;
    TEST(table != NULL);
    TEST(table->global != NULL);
    TEST(table->global == table->local);
    TEST(table->global->used == 0);

    TSData_T *element = malloc(sizeof(TSData_T));
    element->id = malloc(strlen(keys[0]) + 1);
    strcpy(element->id, keys[0]);
    TEST(SymTabLookup(table, "lol") == NULL);
    TEST(SymTabInsertGlobal(table, element));
    TEST(table->global->used == 1);
    TEST((element = SymTabLookup(table, keys[0])) != NULL);
    TEST(strcmp(element->id, keys[0]) == 0);

    SymTabAddLocalBlock(table);
    TEST(table->global->next == table->local);
    element = malloc(sizeof(TSData_T));
    element->id = malloc(strlen(keys[1]) + 1);
    strcpy(element->id, keys[1]);
    TEST(SymTabInsertLocal(table, element));

    TEST(SymTabLookupLocal(table, keys[0]) == NULL);
    TEST((element = SymTabLookupGlobal(table, keys[0])) != NULL);
    TEST(strcmp(element->id, keys[0]) == 0);
    TEST((element = SymTabLookupLocal(table, keys[1])) != NULL);
    TEST(strcmp(element->id, keys[1]) == 0);
    TEST((element = SymTabLookup(table, keys[0])) != NULL);
    TEST(strcmp(element->id, keys[0]) == 0);

    SymTabRemoveLocalBlock(table);
    SymTabDestroy(table);
}

int main()
{
    test();
    if(failures != 0)
    {
        printf("Total tests failed: %d\n", failures);
    }
    else{
        printf("Everything OK\n");
    }
    return 0;
}