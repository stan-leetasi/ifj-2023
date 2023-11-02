/** Projekt IFJ2023
 * @file symtable.h
 * @brief Tabuľka symbolov
 * @author Michal Krulich (xkruli03)
 * @date 22.10.2023
 */

#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define SYM_TYPE_FUNC       'F'
#define SYM_TYPE_INT        'i'
#define SYM_TYPE_INT_NIL    'I'
#define SYM_TYPE_DOUBLE     'd'
#define SYM_TYPE_DOUBLE_NIL 'D'
#define SYM_TYPE_STRING     's'
#define SYM_TYPE_STRING_NIL 'S'
#define SYM_TYPE_VOID       'V'
#define SYM_TYPE_UNKNOWN    'U'

#define SYMTABLE_MAX_SIZE 997 // musí byť prvočíslo

/**
 * @brief Signatúra funkcie
 */
typedef struct func_signature {
    char ret_type;      ///< typ návratovej hodnoty
    char *par_types;    ///< dátové typy parametrov
    char **par_names;   ///< názvy parametrov
    char **par_ids;     ///< identifikátory parametrov používané vo vnútri funkcie
    // func <názov_funkcie> (par_name par_id : par_type, ...) -> ret_type {}
} *func_sig_T;

/**
 * @brief Dátový element / prvok tabuľky symbolov, obsahuje informácie o symbole/identifikátore premennej alebo funkcie
 */
typedef struct TSData {
    char *id;       ///< názov identifikátoru, zároveň kľúč v tabuľke
    char type;      ///< typ premennej/funkcia, používa hodnoty SYM_TYPE_XXX
    bool let;       ///< true znamená premenná let inak var 
    bool init;      ///< true znamená, že je premenná inicializovaná alebo funkcia definovaná
    func_sig_T sig; ///< signatúra funkcie, v prípade premennej sig=NULL
} TSData_T;

/**
 * @brief Dielčí blok/rámec tabuľky symbolov
 */
typedef struct TSBlock {
    size_t used;            ///< počet zaplnených miest
    struct TSBlock *prev;   ///< ukazateľ na predchádzajúci blok
    struct TSBlock *next;   ///< ukazateľ na nasledujúci blok
    TSData_T *array[];      ///< pole ukazateľov na symboly
} TSBlock_T;

/**
 * @brief Tabuľka symbolov
 * @details Ak tabuľka obsahuje len globálny rámec, potom local ukazuje aj na globálny.
 */
typedef struct SymbolsTable {
    TSBlock_T *global;      ///< ukazateľ na globálny blok (prvý blok)
    TSBlock_T *local;       ///< ukazateľ na lokálny blok  (posledný blok)
} SymTab_T;


/**
 * @brief Inicializuje tabuľku symbolov a vloží do nej jeden globálny rámec
 * @return true v prípade úspechu, inak false
*/
bool SymTabInit(SymTab_T *st);

/**
 * @brief Vytvorí a pridá do tabuľky symbolov nový dielčí lokálny rámec/blok
 * @return true v prípade úspechu, inak false
*/
bool SymTabAddLocalBlock(SymTab_T *st);

/**
 * @brief Uvoľní všetok obsah alokovaný v poslednom lokálnom bloku tabuľky a odstráni tento blok z tabuľky.
*/
void SymTabRemoveLocalBlock(SymTab_T *st);

/**
 * @brief Dealokuje všetok obsah tabuľky symbolov.
*/
void SymTabDestroy(SymTab_T *st);

/**
 * @brief Vyhľadá tabuľke symbolov prvú položku s daným kľúčom/symbolom. Vyhľadáva od posledného lokálneho až po globálny.
 * @param st tabuľka symbolov
 * @param key hľadaný klúč/symbol
 * @return položka s daným kľúčom alebo NULL ak sa v tabuľke nenachádza
*/
TSData_T *SymTabLookup(SymTab_T *st, char *key);

/**
 * @brief Vyhľadá len v globálnom bloku tabuľky symbolov položku s daným kľúčom/symbolom.
 * @param st tabuľka symbolov
 * @param key hľadaný klúč/symbol
 * @return položka s daným kľúčom alebo NULL ak sa v tabuľke nenachádza
*/
TSData_T *SymTabLookupGlobal(SymTab_T *st, char *key);

/**
 * @brief Vyhľadá len v lokálnom bloku tabuľky symbolov položku s daným kľúčom/symbolom.
 * @param st tabuľka symbolov
 * @param key hľadaný klúč/symbol
 * @return položka s daným kľúčom alebo NULL ak sa v tabuľke nenachádza
*/
TSData_T *SymTabLookupLocal(SymTab_T *st, char *key);

/**
 * @brief Vloží do globálneho bloku tabuľky symbolov nový prvok.
 * @param st tabuľka symbolov
 * @param elem vkladaný prvok
 * @return true v prípade úspechu, inak false
*/
bool SymTabInsertGlobal(SymTab_T *st, TSData_T *elem);

/**
 * @brief Vloží do posledného lokálneho bloku tabuľky symbolov nový prvok.
 * @param st tabuľka symbolov
 * @param elem vkladaný prvok
 * @return true v prípade úspechu, inak false
*/
bool SymTabInsertLocal(SymTab_T *st, TSData_T *elem);

TSData_T *SymTabBlockLookUp(TSBlock_T *block, char *key);

bool SymTabBlockInsert(TSBlock_T *block, TSData_T *elem);

#endif // ifndef _SYMTABLE_H_
/* Koniec súboru symtable.h */
