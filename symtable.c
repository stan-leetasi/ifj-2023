/** Projekt IFJ2023
 * @file symtable.c
 * @brief Tabuľka symbolov
 * @author Boris Hatala
 * @date 19.11.2023
 * 
 * @todo exit(99), chybové hlášky, komentáre
 */

#include "symtable.h"

unsigned long hashOne(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash;
}

unsigned long hashTwo(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) ^ c;
    }
    return hash;
}

func_sig_T *SymTabCreateFuncSig() {
    func_sig_T *f = malloc(sizeof(func_sig_T));
    if (f == NULL) {
        fprintf(stderr, "SymTabCreateFuncSig() - memory allocation error\n");
        exit(99);
    }
    f->ret_type = SYM_TYPE_UNKNOWN;
    StrInit(&(f->par_types));
    DLLstr_Init(&(f->par_names));
    DLLstr_Init(&(f->par_ids));
    return f;
}

TSData_T *SymTabCreateElement(char *key)
{
    TSData_T *elem = malloc(sizeof(TSData_T));
    if(elem == NULL) {
        fprintf(stderr, "SymTabCreateElement() - memory allocation error\n");
        exit(99);
    }
    elem->id = malloc(sizeof(char)*(strlen(key)+1));
    if(elem->id == NULL) {
        free(elem);
        fprintf(stderr, "SymTabCreateElement() - memory allocation error\n");
        exit(99);
    }
    elem->type = SYM_TYPE_UNKNOWN;
    strcpy(elem->id, key);
    StrInit(&(elem->codename));
    return elem; 
}

void SymTabDestroyElement(TSData_T *elem) {
    if(elem != NULL) {
        if(elem->type == SYM_TYPE_FUNC) {
            StrDestroy(&(elem->sig->par_types));
            DLLstr_Dispose(&(elem->sig->par_names));
            DLLstr_Dispose(&(elem->sig->par_ids));
            free(elem->sig);
        }
        free(elem->id);
        StrDestroy(&(elem->codename));
        free(elem);
    }
}

void SymTabInit(SymTab_T *st) {

    st -> global = malloc(sizeof (TSBlock_T) + sizeof(TSData_T*) * SYMTABLE_MAX_SIZE);

    if(st -> global == NULL) {
        fprintf(stderr, "SymTabInit() - memory allocation error\n");
        exit(99);
    }

    st -> global -> used = 0;
    st -> global -> prev = NULL;
    st -> global -> next = NULL;
    st -> global -> has_return = false;

    // NULL značí prázdne (voľné) miesto v tabuľke
    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        st -> global -> array[i] = NULL;
    }

    st -> local = st -> global;
}

void SymTabAddLocalBlock(SymTab_T *st) {
    if (st == NULL) {
        fprintf(stderr, "SymTabAddLocalBlock() - symbol table not initialized\n");
        exit(99);
    }
    
    TSBlock_T *newBlock = malloc(sizeof (TSBlock_T) + sizeof(TSData_T*) * SYMTABLE_MAX_SIZE);

    if(newBlock == NULL) { 
        fprintf(stderr, "SymTabAddLocalBlock() - new block memory allocation error\n");
        exit(99);
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
    newBlock -> has_return = false;

    st -> local = newBlock;
}

void SymTabRemoveLocalBlock(SymTab_T *st) {
    TSBlock_T *currentLocal = st->local;
    st->local = currentLocal->prev;

    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        TSData_T *data = currentLocal->array[i];
        if(data != NULL) {
            SymTabDestroyElement(data);
        }
    }   

    free(currentLocal); 
}

void SymTabDestroy(SymTab_T *st) {
    while (st->local != NULL) {
        SymTabRemoveLocalBlock(st);
    }

    st->global = NULL;
    st->local = NULL;
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

void SymTabInsertGlobal(SymTab_T *st, TSData_T *elem) {

    if(st -> global == NULL) {
        fprintf(stderr, "SymTabInsertGlobal() - global table is empty\n");
        exit(99);
    }

    SymTabBlockInsert(st->global, elem);
}

void SymTabInsertLocal(SymTab_T *st, TSData_T *elem) {

    if(st -> local == NULL) {
            fprintf(stderr, "SymTabInsertLocal() - local table is empty\n");
            exit(99);
        }

    SymTabBlockInsert(st->local, elem);
}

bool SymTabCheckLocalReturn(SymTab_T *st) {
    return st->local->has_return;
}

void SymTabModifyLocalReturn(SymTab_T *st, bool value) {
    st->local->has_return = value;
}

TSData_T *SymTabBlockLookUp(TSBlock_T *block, char *key) {

    size_t h1 = hashOne(key) % SYMTABLE_MAX_SIZE;
    size_t h2 = (hashTwo(key) % (SYMTABLE_MAX_SIZE - 1)) + 1;
    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++) {
        size_t index = (h1 + i * h2) % SYMTABLE_MAX_SIZE;
        if (block->array[index] == NULL) {
            return NULL;
        }
        else {
            if (strcmp(block->array[index]->id, key) == 0) {
                return block->array[index];
            }
        }
    }
    
    return NULL;
}

void SymTabBlockInsert(TSBlock_T *block, TSData_T *elem) {

    if(block->used + 1 == SYMTABLE_MAX_SIZE) {
        fprintf(stderr, "SymTabBlockInsert() - symbol table is full\n");
        exit(99);
    }

    size_t h1 = hashOne(elem->id) % SYMTABLE_MAX_SIZE;
    size_t h2 = (hashTwo(elem->id) % (SYMTABLE_MAX_SIZE - 1)) + 1;
    for (size_t i = 0; i < SYMTABLE_MAX_SIZE; i++)
    {
        size_t index = (h1 + i * h2) % SYMTABLE_MAX_SIZE;
        if (block->array[index] == NULL) {
            block->array[index] = elem;
            block->used++;
            return;
        }
    }
}
/* Koniec súboru symtable.c */
