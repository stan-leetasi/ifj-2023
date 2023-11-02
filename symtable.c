/** Projekt IFJ2023
 * @file symtable.c
 * @brief Tabuľka symbolov
 * @author Boris Hatala
 * @date 30.10.2023
 * 
 * @todo Uvoľnenie celej tabuľky, refaktorizácia kódu (private metódy pre TSBlock_T)
 */

#include "symtable.h"

unsigned long hashOne(char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
    while ((c = *str++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    // printf("hashone\n");
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
    // printf("hashtwo\n");
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
        TSBlock_T *currentLocal = st->local;
        st->local = currentLocal->prev;

        for (size_t i = 0; i < SYMTABLE_MAX_SIZE - 1; i++) {
            TSData_T *data = currentLocal->array[i];
                if(data != NULL) {
                    free(data->id);
                    free(data); 
                }
        }
        free(currentLocal); 
}

void SymTabDestroy(SymTab_T *st) {
    while (st->local != NULL) {
        SymTabRemoveLocalBlock(st);
    }

    // for(size_t i = 0; i < SYMTABLE_MAX_SIZE - 1; i++) {
    //     TSData_T *data = st->global->array[i];
    //     if(data != NULL) {
    //         free(data->id);
    //         free(data);
    //     }
    // }
    // free(st->global);
    free(st);
}

TSData_T *SymTabLookup(SymTab_T *st, char *key) {
    if (st == NULL || st->global == NULL) {
        return NULL;
    }

    TSBlock_T *currentBlock = st->local;
    TSData_T *result = NULL;
    while (currentBlock != NULL) {
        result = SymTabBlockLookUp(currentBlock, key);
        if(result != NULL) {
            return result;
        }
        currentBlock = currentBlock->prev;
    }

    // result = SymTabBlockLookUp(st->global, key);
    // if(result != NULL) {
    //     return result;
    // }
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

    TSData_T *result = SymTabBlockLookUp(st->global, key);
    return result;
}

TSData_T *SymTabLookupLocal(SymTab_T *st, char *key) {

    if(st->local == NULL) {
        return NULL;
    }
    TSData_T *result = SymTabBlockLookUp(st->local, key);
    return result;
}

bool SymTabInsertGlobal(SymTab_T *st, TSData_T *elem) {

    if(st -> global == NULL) {
        return false;
    }
    // printf("kokot\n");

    bool result = SymTabBlockInsert(st->global, elem);
    return result;
}

bool SymTabInsertLocal(SymTab_T *st, TSData_T *elem) {

    if(st -> local == NULL) {
            return false;
        }
    bool result = SymTabBlockInsert(st->local, elem);
    return result;
}

TSData_T *SymTabBlockLookUp(TSBlock_T *block, char *key) {

    size_t h1 = hashOne(key) % SYMTABLE_MAX_SIZE;

    if(block->array[h1] != NULL) {
        if(strcmp(block->array[h1]->id, key) == 0)
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

    block->used++;
    if(block->used + 1 == SYMTABLE_MAX_SIZE) {
        return false;
    }

    size_t h1 = hashOne(elem->id) % SYMTABLE_MAX_SIZE;
    
    if(block->array[h1] == NULL) {
        block->array[h1] = elem;
        return true;
    }
    else if(block->array[h1] != NULL) {
        size_t h2 = (hashTwo(elem->id) % (SYMTABLE_MAX_SIZE - 1)) + 1;
        // printf("chuj2\n");
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
