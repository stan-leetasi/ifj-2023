/** Projekt IFJ2023
 * @file dll.h
 * @brief Double linked list, dvojsmerný zoznam na generované inštrukcie
 * @author Michal Krulich
 * @date 25.10.2023
 * @todo Pridať doxygen komentáre
 */

#ifndef _DLL_H_
#define _DLL_H_

#include <stdbool.h>
#include "strR.h"

/**
 * @brief Prvok DLL
*/
typedef struct DLLstr_element {
    str_T string;                   ///< uchovávaný reťazec
    struct DLLstr_element *prev;    ///< predchádzajúci prvok
    struct DLLstr_element *next;    ///< nasledujúci prvok
} *DLLstr_el_ptr;

/**
 * @brief DLL
*/
typedef struct DLL_str_struct {
	DLLstr_el_ptr first;     ///< prvý prvok
	DLLstr_el_ptr active;
	DLLstr_el_ptr last;
} DLLstr_T;

void DLLstr_ElementDestroy(DLLstr_el_ptr elem);

void DLLstr_Init( DLLstr_T *list );
bool DLLstr_IsActive( DLLstr_T *list );
void DLLstr_First( DLLstr_T *list );
void DLLstr_Last( DLLstr_T *list );
void DLLstr_Next( DLLstr_T *list );
void DLLstr_Previous( DLLstr_T *list );
bool DLLstr_GetFirst( DLLstr_T *list, str_T *string);
bool DLLstr_GetLast( DLLstr_T *list, str_T *string);
bool DLLstr_GetValue( DLLstr_T *list, str_T *string);
bool DLLstr_InsertFirst( DLLstr_T *list, str_T string);
bool DLLstr_InsertLast( DLLstr_T *list, str_T string);
bool DLLstr_InsertAfter( DLLstr_T *list, str_T string);
bool DLLstr_InsertBefore( DLLstr_T *list, str_T string);
void DLLstr_DeleteFirst( DLLstr_T *list );
void DLLstr_DeleteLast( DLLstr_T *list );
void DLLstr_DeleteAfter( DLLstr_T *list );
void DLLstr_DeleteBefore( DLLstr_T *list );
void DLLstr_Dispose( DLLstr_T *list );

#endif // ifndef _DLL_H_
/* Koniec súboru dll.h */
