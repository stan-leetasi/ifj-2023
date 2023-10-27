/** Projekt IFJ2023
 * @file symtable.c
 * @brief Tabuľka symbolov
 * @author Boris Hatala
 * @date 26.10.2023
 */

#include "symtable.h"

bool SymTabInit(SymTab_T *st) {

    st -> global = (TSBlock_T*) malloc(sizeof(TSBlock_T));

    if(st -> global == NULL) {
        return false;
    }

    st -> global -> used = 0;
    st -> global -> prev = NULL;
    st -> global -> next = NULL;

    st -> global -> array = (TSData_T*) malloc(sizeof(TSData_T) * SYMTABLE_MAX_SIZE);

    if(st -> global -> array == NULL) {
        free(st -> global);
        return false;
    }

    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        st -> global -> array[i].id = NULL;
        st -> global -> array[i].type = SYM_TYPE_UNKNOWN;
        st -> global -> array[i].let = false;
        st -> global -> array[i].init = false;
        st -> global -> array[i].sig = NULL;
    }

    st -> local = st -> global;

    return true;
}

bool SymTabAddLocalBlock(SymTab_T *st) {
    TSBlock_T *newBlock = (TSBlock_T*) malloc(sizeof(TSBlock_T));

    if(newBlock == NULL) { 
        return false;
    }

    newBlock -> used = 0;
    st -> global -> prev = st -> local;
    st -> global -> next = NULL;

    newBlock -> array = (TSData_T*) malloc(sizeof(TSData_T) * SYMTABLE_MAX_SIZE);

    if(newBlock -> array == NULL) {
        free(newBlock);
        return false;
    }

    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        newBlock -> array[i].id = NULL;
        newBlock -> array[i].type = SYM_TYPE_UNKNOWN;
        newBlock -> array[i].let = false;
        newBlock -> array[i].init = false;
        newBlock -> array[i].sig = NULL;
    }

    if(st -> local != NULL) {
        st -> local -> next = newBlock;
    }

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

    TSBlock_T *currentBlock = st -> local;

    while(currentBlock != NULL) {

        for(size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
            if(currentBlock -> array[i].id != NULL && strcmp(currentBlock -> array[i].id, key) == 0) {
                return &currentBlock -> array[i];
            }
        }
        currentBlock = currentBlock -> prev;
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

    TSBlock_T *currentBlock = st -> global;

    for(size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        if(currentBlock -> array[i].id != NULL && strcmp(currentBlock -> array[i].id, key) == 0) {
            return &currentBlock -> array[i];
        }
    }
    currentBlock = currentBlock -> prev;

    return NULL;
}

TSData_T *SymTabLookupLocal(SymTab_T *st, char *key) {

    TSBlock_T *currentBlock = st -> local;

    for(size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        if(currentBlock -> array[i].id != NULL && strcmp(currentBlock -> array[i].id, key) == 0) {
            return &currentBlock -> array[i];
        }
    }
    currentBlock = currentBlock -> prev;

    return NULL;

}

bool SymTabInsertGlobal(SymTab_T *st, TSData_T *elem) {

    if(st -> global == NULL) {
        return false;
    }

    //iterate through global block until we find an empty slot
    for(size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        if (st -> global -> array[i].id == NULL) {
            st -> global -> array[i] = *elem;
            return true;
        }    
    }

    return false;
}

bool SymTabInsertLocal(SymTab_T *st, TSData_T *elem) {

    if(st -> local == NULL) {
            return false;
        }

    //iterate through local block until we find an empty slot
    for(size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        if (st -> local -> array[i].id == NULL) {
            st -> local -> array[i] = *elem;
            return true;
        }    
    }

    return false;
}











/* Koniec súboru symtable.c */
