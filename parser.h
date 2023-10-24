/** Projekt IFJ2023
 * @file parser.h
 * @brief Syntaktický a sémantický anayzátor
 * @author 
 * @date 
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdlib.h>
#include "symtable.h"
#include "scanner.h"

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
 * @brief Aktuálny načítaný token
*/
extern token_T *tkn;

/**
 * @brief Tabuľka symbolov
*/
extern SymTab_T *symt;

/**
 * 
*/
// extern DLL generated_code

/**
 * @brief Hlavný cyklus parsera, funkcia rekurzívne spracúva jednotlivé príkazy
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parse();

#endif // ifndef _PARSER_H_
/* Koniec súboru parser.h */
