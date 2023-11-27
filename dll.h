/** Projekt IFJ2023
 * @file dll.h
 * @brief Double linked list, dvojsmerný zoznam na generované inštrukcie
 * @author Michal Krulich (xkruli03)
 * @date 27.11.2023
 */

#ifndef _DLL_H_
#define _DLL_H_

#include <stdbool.h>
#include "strR.h"

 /**
  * @brief Prvok DLL
 */
typedef struct DLLstr_element {
    char* string;                   ///< uchovávaný reťazec
    struct DLLstr_element* prev;    ///< predchádzajúci prvok
    struct DLLstr_element* next;    ///< nasledujúci prvok
} *DLLstr_el_ptr;

/**
 * @brief DLL
*/
typedef struct DLL_str_struct {
    DLLstr_el_ptr first;     ///< prvý prvok
    DLLstr_el_ptr active;    ///< aktívny prvok
    DLLstr_el_ptr last;      ///< posledný prvok
} DLLstr_T;

/**
 * @brief Dealokuje prvok zoznamu
 * @param elem ukazateľ na prvok
*/
void DLLstr_ElementDestroy(DLLstr_el_ptr elem);

/**
 * @brief Inicializuje zoznam
 * @param list dll zoznam
*/
void DLLstr_Init(DLLstr_T* list);

/**
 * @brief Zistí, či je zoznam aktívny
 * @param list dll zoznam
 * @return true ak je zoznam aktívny, inak false
*/
bool DLLstr_IsActive(DLLstr_T* list);

/**
 * @brief Nastaví aktivitu zoznamu na prvý prvok, v prípade prázdneho zoznamu, zoznam ostáva neaktívny
 * @param list dll zoznam
*/
void DLLstr_First(DLLstr_T* list);

/**
 * @brief Nastaví aktivitu zoznamu na posledný prvok, v prípade prázdneho zoznamu, zoznam ostáva neaktívny
 * @param list dll zoznam
*/
void DLLstr_Last(DLLstr_T* list);

/**
 * @brief Nastaví aktivitu zoznamu na nasledujúci prvok
 * @param list dll zoznam
*/
void DLLstr_Next(DLLstr_T* list);

/**
 * @brief Nastaví aktivitu zoznamu na predchádzajúci prvok
 * @param list dll zoznam
*/
void DLLstr_Previous(DLLstr_T* list);

/**
 * @brief Získanie reťazca zo začiatku zoznamu
 * @param list dll zoznam
 * @param string Odkaz na inicializovaný str_T, do ktorého sa zapíše reťazec zo zoznamu
 * @return true v prípade úspechu, inak false (zoznam je prázdny)
*/
bool DLLstr_GetFirst(DLLstr_T* list, str_T* string);

/**
 * @brief Získanie reťazca z konca zoznamu
 * @param list dll zoznam
 * @param string Odkaz na inicializovaný str_T, do ktorého sa zapíše reťazec zo zoznamu
 * @return true v prípade úspechu, inak false (zoznam je prázdny)
*/
bool DLLstr_GetLast(DLLstr_T* list, str_T* string);

/**
 * @brief Získanie reťazca z aktívneho prvku zoznamu
 * @param list dll zoznam
 * @param string Odkaz na inicializovaný str_T, do ktorého sa zapíše reťazec zo zoznamu
 * @return true v prípade úspechu, inak false (zoznam je neaktívny)
*/
bool DLLstr_GetValue(DLLstr_T* list, str_T* string);

/**
 * @brief Vloží na začiatok zoznamu novo alokovanú kópiu poskytnutého reťazca
 * @param list zoznam
 * @param s reťazec, ktorý má byť vložený do zoznamu
*/
void DLLstr_InsertFirst(DLLstr_T* list, char* s);

/**
 * @brief Vloží na koniec zoznamu novo alokovanú kópiu poskytnutého reťazca
 * @param list zoznam
 * @param s reťazec, ktorý má byť vložený do zoznamu
*/
void DLLstr_InsertLast(DLLstr_T* list, char* s);

/**
 * @brief Vloží za aktívny prvok novo alokovanú kópiu poskytnutého reťazca
 * @param list zoznam
 * @param s reťazec, ktorý má byť vložený do zoznamu
*/
void DLLstr_InsertAfter(DLLstr_T* list, char* s);

/**
 * @brief Vloží pred aktívny prvok novo alokovanú kópiu poskytnutého reťazca
 * @param list zoznam
 * @param s reťazec, ktorý má byť vložený do zoznamu
*/
void DLLstr_InsertBefore(DLLstr_T* list, char* s);

/**
 * @brief Vymaže prvý prvok zo zoznamu, v prípade prázdneho zoznamu bez efektu
 * @param list zoznam
*/
void DLLstr_DeleteFirst(DLLstr_T* list);

/**
 * @brief Vymaže posledný prvok zo zoznamu, v prípade prázdneho zoznamu bez efektu
 * @param list zoznam
*/
void DLLstr_DeleteLast(DLLstr_T* list);

/**
 * @brief Vymaže prvok za aktívnym prvkom zoznamu, v prípade neaktívneho zoznamu bez efektu
 * @param list zoznam
*/
void DLLstr_DeleteAfter(DLLstr_T* list);

/**
 * @brief Vymaže prvok pred aktívnym prvkom zoznamu, v prípade neaktívneho zoznamu bez efektu
 * @param list zoznam
*/
void DLLstr_DeleteBefore(DLLstr_T* list);

/**
 * @brief Vyprázdni zoznam a inicializuje ho na prázdny.
 * @param list zoznam
*/
void DLLstr_Dispose(DLLstr_T* list);

/**
 * Môže zmeniť aktivitu zoznamu.
 * 
 * @brief Vypíše obsah zoznamu na stdout. Za každou položkou sa vytlačí znak nového riadku.
 * @param list Zoznam
*/
void DLLstr_printContent(DLLstr_T *list);

#endif // ifndef _DLL_H_
/* Koniec súboru dll.h */
