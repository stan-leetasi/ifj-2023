/** Projekt IFJ2023
 * @file symtable.c
 * @brief Tabuľka symbolov
 * @author Boris Hatala
 * @date 26.10.2023
 */

#include "symtable.h"

unsigned long hashOne(char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

unsigned long hashTwo(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) ^ c;
    }

    return hash;
}

bool SymTabInit(SymTab_T *st) {

    st -> global = malloc(sizeof (TSBlock_T) + sizeof(TSData_T*) * SYMTABLE_MAX_SIZE);

    if(st -> global == NULL) {
        return false;
    }

    st -> global -> used = 0;
    st -> global -> prev = NULL;
    st -> global -> next = NULL;

    // NULL značí prázdne (voľné) miesto v tabuľke
    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        st -> global -> array[i] = NULL;
    }

    st -> local = st -> global;

    return true;
}

bool SymTabAddLocalBlock(SymTab_T *st) {
    if (st == NULL) {
        return false;
    }
    
    TSBlock_T *newBlock = malloc(sizeof (TSBlock_T) + sizeof(TSData_T*) * SYMTABLE_MAX_SIZE);

    if(newBlock == NULL) { 
        return false;
    }

    newBlock -> used = 0;

    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        newBlock -> array[i] = NULL;
    }

    if(st -> local != NULL) {
        st -> local -> next = newBlock;
    }

    newBlock -> prev = st -> local;
    newBlock -> next = NULL;

    st -> local = newBlock;

    return true;
}

void SymTabRemoveLocalBlock(SymTab_T *st) {
    if (st->local != NULL) {

        free(st->local->array);
        
        if (st->local->prev != NULL) {
            st->local->prev->next = NULL;
        }
        
        st->local = st->local->prev;
        
        free(st->local->next);
    }
}

void SymTabDestroy(SymTab_T *st) {

    TSBlock_T *currentBlock = st->global;

    while (currentBlock != NULL) {
        TSBlock_T *nextBlock = currentBlock->next;
        
        free(currentBlock->array);
        
        free(currentBlock);
        
        currentBlock = nextBlock;
    }
    
    st->global = NULL;
    st->local = NULL;
}

TSData_T *SymTabLookup(SymTab_T *st, char *key) {
    if (st == NULL || st->global == NULL) {
        return NULL;
    }

    if(SymTabBlockLookUp(st->global, key) != NULL) {
        return SymTabBlockLookUp(st->global, key);
    }

    // Iterate through all local blocks and look for the symbol in each
    TSBlock_T *currentBlock = st->local;
    while (currentBlock != NULL) {
        for (size_t i = 0; i < currentBlock->used; i++) {
            if (currentBlock->array[i] != NULL && strcmp(currentBlock->array[i]->id, key) == 0) {
                return currentBlock->array[i];
            }
        }
        currentBlock = currentBlock->prev;
    }

    return NULL;
    
}

/**
 * @brief Vyhľadá len v globálnom bloku tabuľky symbolov položku s daným kľúčom/symbolom.
 * @param st tabuľka symbolov
 * @param key hľadaný klúč/symbol
 * @return položka s daným kľúčom alebo NULL ak sa v tabuľke nenachádza
*/
TSData_T *SymTabLookupGlobal(SymTab_T *st, char *key) {

    if(st->global == NULL) {
        return NULL;
    }

    return SymTabBlockLookUp(st->global, key);
}

TSData_T *SymTabLookupLocal(SymTab_T *st, char *key) {

    if(st->local == NULL) {
        return NULL;
    }

    return SymTabBlockLookUp(st->local, key);
}

bool SymTabInsertGlobal(SymTab_T *st, TSData_T *elem) {

    if(st -> global == NULL) {
        return false;
    }

    return SymTabBlockInsert(st->global, elem);
}

bool SymTabInsertLocal(SymTab_T *st, TSData_T *elem) {

    if(st -> local == NULL) {
            return false;
        }

    return SymTabBlockInsert(st->local, elem);
}

TSData_T *SymTabBlockLookUp(TSBlock_T *block, char *key) {

    size_t h1 = hashOne(key) % SYMTABLE_MAX_SIZE;

    if(block->array[h1] != NULL && strcmp(block->array[h1]->id, key) == 0) {
        return block->array[h1];
    }

    size_t h2 = (hashTwo(key) % (SYMTABLE_MAX_SIZE - 1)) + 1;
    for (size_t i = 1; i < SYMTABLE_MAX_SIZE; i++) {
        size_t index = (h1 + i * h2) % SYMTABLE_MAX_SIZE;
        if (block->array[index] != NULL && strcmp(block->array[index]->id, key) == 0) {
            return block->array[index];
        }
    }

    return NULL;
}

bool SymTabBlockInsert(TSBlock_T *block, TSData_T *elem) {

    if(block->used + 1 == SYMTABLE_MAX_SIZE) {
        return false;
    }

    size_t h1 = hashOne(elem->id) % SYMTABLE_MAX_SIZE;
    if(block->array[h1] != NULL) {
        size_t h2 = (hashTwo(elem->id) % (SYMTABLE_MAX_SIZE - 1)) + 1;
        for (size_t i = 1; i < SYMTABLE_MAX_SIZE; i++)
        {
            if (block->array[(h1 + i*h2)%SYMTABLE_MAX_SIZE] == NULL) {
                block->array[(h1 + i*h2)%SYMTABLE_MAX_SIZE] = elem;
                return true;
            }
        }
    }
    return false;
}

/* Koniec súboru symtable.c */
