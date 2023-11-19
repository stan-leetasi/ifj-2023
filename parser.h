/** Projekt IFJ2023
 * @file parser.h
 * @brief Syntaktický a sémantický anayzátor
 * @author Michal Krulich (xkruli03)
 * @date 17.11.2023
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdlib.h>
#include "symtable.h"
#include "scanner.h"
#include "dll.h"
#include "strR.h"

#define COMPILATION_OK  0 ///< Preklad bez chýb
#define LEX_ERR         1 ///< Chybný lexém 
#define SYN_ERR         2 ///< Syntaktická chyba - nesprávny typ tokenu
#define SEM_ERR_REDEF   3 ///< Sémantická chyba - nedefinovaná funkcia, redefinícia premennej
#define SEM_ERR_FUNC    4 ///< Nesprávne volanie funkcie či nesprávny typ návratovej hodnoty
#define SEM_ERR_UNDEF   5 ///< nedefinová alebo neinicializovaná premenná
#define SEM_ERR_RETURN  6 ///< chybný výraz v return
#define SEM_ERR_TYPE    7 ///< Typová nekompatibilita
#define SEM_ERR_UKN_T   8 ///< Nie je možné odvodiť typ
#define SEM_ERR_OTHER   9 ///< ostatné sémantické chyby
#define COMPILER_ERROR  99 ///< Chyba prekladača

 /**
  * @brief Vykoná zadanú operáciu a v prípade návratovej hodnotej rôznej od nuly (COMPILATION_OK),
  *        použije ju ako návratovú hodnotu aktuálnej funkcie.
 */
#define TRY_OR_EXIT(operation)              \
    do {                                    \
    int error_code = (operation);           \
    if (error_code != 0) return error_code; \
    } while (0)

 /**
  * @brief Aktuálny načítaný token
 */
extern token_T* tkn;

/**
 * @brief Tabuľka symbolov
*/
extern SymTab_T symt;

/**
 * @brief Indikuje, či sa aktuálne spracúva kód vo vnútri funkcie.
 * @details Podľa toho sa generovaný kód ukladá buď do code_fn alebo code_main.
*/
extern bool parser_inside_fn_def;

/**
 * @brief Zistí kompatibilitu priradenia dvoch typov
*/
bool isCompatibleAssign(char dest, char src);

/**
 * @brief Uvoľní aktuálne načítaný token v globálnej premennej tkn a nahradí ho novým zo scannera
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int nextToken();

/**
 * @brief Uloží token v globálnej premennej tkn do úschovňe skenera a prepíše tkn na NULL
*/
void saveToken();

/**
 * @brief Inicializácia dátových štruktúr parsera
 * @return true v prípade úspechu, inak false
*/
bool initializeParser();

/**
 * Stav tkn:
 *  - pred volaním: načítaný token
 *  - po volaní:    rôzny
 *
 * @brief Hlavná časť parsera, funkcia spracúva jeden <STAT>
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parse();

/**
 * @brief Skontroluje, či boli definované všetky volané funkcie
 * @return 0 ak je všetko v poriadku, inak číslo chyby
*/
int checkIfAllFnDef();

/**
 * @brief Vypíše na stdout vygenerovaný IFJcode23.
*/
void printOutCompiledCode();

/**
 * @brief Uvoľní všetky hlavné zdroje využívané prekladačom (parsera)
*/
void destroyParser();

#endif // ifndef _PARSER_H_
/* Koniec súboru parser.h */
