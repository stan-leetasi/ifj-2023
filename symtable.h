/** Projekt IFJ2023
 * @file symtable.h
 * @brief Tabuľka symbolov
 * @author Michal Krulich (xkruli03)
 * @date 22.10.2023
 */

#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include "strR.h"
#include <stdbool.h>

#define SYM_TYPE_FUNC       'F'
#define SYM_TYPE_INT        'i'
#define SYM_TYPE_INT_NIL    'I'
#define SYM_TYPE_DOUBLE     'd'
#define SYM_TYPE_DOUBLE_NIL 'D'
#define SYM_TYPE_STRING     's'
#define SYM_TYPE_STRING_NIL 'S'
#define SYM_TYPE_VOID       'V'
#define SYM_TYPE_UNKNOWN    'U'

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
    str_T id;       ///< názov identifikátoru, zároveň kľúč v tabuľke
    char type;      ///< typ premennej/funkcia, používa hodnoty SYM_TYPE_XXX
    bool let;       ///< true znamená premenná let inak var 
    bool init;      ///< true znamená, že je premenná inicializovaná alebo funkcia definovaná
    func_sig_T sig;///< signatúra funkcie, v prípade premennej sig=NULL
} TSData_T;

/**
 * @brief Dielčí blok/rámec tabuľky symbolov
 */
typedef struct TSBlock {
    size_t used;            ///< počet zaplnených miest
    struct TSBlock *prev;   ///< ukazateľ na predchádzajúci blok
    struct TSBlock *next;   ///< ukazateľ na nasledujúci blok
    TSData *array;          ///< pole pre symboly
} TSBlock_T;

/**
 * @brief Tabuľka symbolov
 */
typedef struct SymbolsTable {
    TSBlock_T *global;      ///< ukazateľ na globálny blok (prvý blok)
    TSBlock_T *local;       ///< ukazateľ na lokálny blok  (posledný blok)
} *SymTab_T;

#endif // ifndef _SYMTABLE_H_
/* Koniec súboru symtable.h */
