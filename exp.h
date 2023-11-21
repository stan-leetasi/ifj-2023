/** Projekt IFJ2023
 * @file exp.h
 * @brief Precedenčná syntaktická a sémantická analýza výrazov s generovaním cieľového kódu
 * @author Michal Krulich (xkruli03)
 * @date 11.10.2023
 */

#ifndef _EXP_H_
#define _EXP_H_

#include "parser.h"

/**
 * Táto funkcia:
 *  - žiada o tokeny dokým je možné vytvoriť zmysluplný výraz.
 *  - prevádza infixový výraz na postfixový
 *  - na základe postfixového výrazu generuje cieľový kód, pričom kontroluje
 *    sémantiku za pomoci tabuľky symbolov:
 *          - či sú premenné deklarované a inicializované
 *          - či sedia dátové typy operandov
 *  - výsledok je v cieľovom kóde uložený na vrchol zásobníka
 *  
 *  Funkcia očakáva v globálnej premennej tkn (súbor parser.h/c) načítaný prvý token
 *  a zanechá túto premennú prázdnu (NULL), resp. uloží posledný token nepatriaci do
 *  výrazu naspäť skeneru.
 *
 * @brief Precedenčná syntaktická analýza výrazov
 * @param result_type Dátový typ výsledku výrazu
 * @param literal Indikuje, či je výraz literál, aby mohol byť v prípade potreby implicitne konvertovaný na double
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseExpression(char* result_type, bool *literal);

#endif // ifndef _EXP_H_
/* Koniec súboru exp.h */
