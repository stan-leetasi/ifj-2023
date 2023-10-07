/** Projekt IFJ2023
 * @file strR.h
 * @brief Resizable string - reťazec s automatickou realokáciu veľkosti
 * @author Michal Krulich (xkruli03)
 * @date 03.10.2023
 */

#ifndef _STRR_H_
#define _STRR_H_

#include <stdlib.h>

#define STR_INIT_SIZE 16 ///< veľkosť novo inicializovaného str_T

/**
 * @brief reťazec s automatickou realokáciu veľkosti
 * @details Na prácu s reťazcom treba používať vytvorené metódy nižšie, ktoré automaticky
 * realokujú potrebné miesto pre reťazec. (Vždy zdvojnásobiť veľkosť.)
*/
typedef struct stringR {
    char *data; ///< alokované pole znakov
    size_t size; ///< alokovaná veľkosť
} str_T;

/**
 * @brief Inicializuje reťazec s kapacitou STR_INIT_SIZE a zapíše do neho prázdny reťazec, znak \0
*/
void StrInit(str_T *s);

/**
 * @brief Dealokuje dátovú štruktúru str_T
*/
void StrDestroy(str_T *s);

/**
 * @brief Prečíta reťazec
 * @return ukazateľ na reťazec, s->data
*/
char *StrRead(str_T *s);

/**
 * @brief Pridá znak na koniec reťazca
*/
void StrAppend(str_T *s, char c);

/**
 * @brief Zaplní dest s obsahom klasického reťazca
*/
void StrFillWith(str_T *dest, char *src);

/**
 * @brief Spojí dva reťazce dest.src a výsledok uloží do dest.
*/
void StrCat(str_T *dest, str_T *src);

#endif // ifndef _STRR_H_
/* Koniec súboru strR.h */
