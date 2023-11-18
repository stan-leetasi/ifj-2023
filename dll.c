/** Projekt IFJ2023
 * @file dll.c
 * @brief Double linked list, dvojsmerný zoznam na generované inštrukcie
 * @author Michal Krulich (xkruli03)
 * @date 17.11.2023
 */

#include <stdlib.h>
#include <stdio.h>
#include "dll.h"

 /**
  * @brief Makro pre alokáciu novej kópie reťazca do dst
  * @param dest Deklarovaná premenná typu char*
  * @param src Reťazec, ktorého kópia sa bude vytvárať
  * @param in_case_of_failure Príkaz, ktorý sa vykonáva v prípade zlyhania alokácie pamäte
  * @return Vracia false ak zlyhá alokácia
 */
#define TRY_DEEPCOPY_STRING(dest, src, in_case_of_failure) \
	do \
	{ \
		(dest) = malloc(sizeof(char) * (strlen(src) + 1)); \
		if ((dest) == NULL) return ((in_case_of_failure), false); \
		strcpy((dest), (src)); \
	} while (0) 


void DLLstr_ElementDestroy(DLLstr_el_ptr elem) {
	free(elem->string);
	free(elem);
}

void DLLstr_Init(DLLstr_T* list) {
	list->first = NULL;
	list->active = NULL;
	list->last = NULL;
}

bool DLLstr_IsActive(DLLstr_T* list) {
	return list->active != NULL;
}

void DLLstr_First(DLLstr_T* list) {
	list->active = list->first;
}

void DLLstr_Last(DLLstr_T* list) {
	list->active = list->last;
}

void DLLstr_Next(DLLstr_T* list) {
	if (list->active != NULL) {
		list->active = list->active->next;
	}
}

void DLLstr_Previous(DLLstr_T* list) {
	if (list->active != NULL) {
		list->active = list->active->prev;
	}
}

bool DLLstr_GetFirst(DLLstr_T* list, str_T* string) {
	if (list->first == NULL) {
		return false;
	}
	StrFillWith(string, list->first->string);
	return true;
}

bool DLLstr_GetLast(DLLstr_T* list, str_T* string) {
	if (list->last == NULL) {
		return false;
	}
	StrFillWith(string, list->last->string);
	return true;
}

bool DLLstr_GetValue(DLLstr_T* list, str_T* string) {
	if (list->active == NULL) {
		return false;
	}
	StrFillWith(string, list->active->string);
	return true;
}

bool DLLstr_InsertFirst(DLLstr_T* list, char* s) {
	DLLstr_el_ptr element = malloc(sizeof(struct DLLstr_element));
	if (element == NULL) { // chyba alokácie pamäte
		return false;
	}
	TRY_DEEPCOPY_STRING(element->string, s, free(element));
	element->prev = NULL; // prvý prvok nemá predchodcu
	if (list->first == NULL) { // zoznam je prázdny, nový prvok sa stáva prvým aj posledným
		list->first = element;
		list->last = element;
		element->next = NULL;
	}
	else {
		list->first->prev = element;
		element->next = list->first;
		list->first = element;
	}
	return true;
}

bool DLLstr_InsertLast(DLLstr_T* list, char* s) {
	DLLstr_el_ptr element = malloc(sizeof(struct DLLstr_element));
	if (element == NULL) { // chyba alokácie pamäte
		return false;
	}
	TRY_DEEPCOPY_STRING(element->string, s, free(element));
	element->next = NULL; // posledný prvok nemá nasledovníka
	if (list->first == NULL) { // zoznam je prázdny, nový prvok sa stáva prvým aj posledným
		list->first = element;
		list->last = element;
		element->prev = NULL;
	}
	else {
		// previažem posledný prvok s novým tak, že nový sa stane posledným
		list->last->next = element;
		element->prev = list->last;
		list->last = element;
	}
	return true;
}

bool DLLstr_InsertAfter(DLLstr_T* list, char* s) {
	if (list->active == NULL) { // neaktívny zoznam
		return false;
	}
	DLLstr_el_ptr element = malloc(sizeof(struct DLLstr_element)); // nový vkladaný prvok
	if (element == NULL) {
		return false;
	}
	TRY_DEEPCOPY_STRING(element->string, s, free(element));
	if (list->last == list->active) { // aktívny prvok je posledný v zozname
		list->active->next = element;
		element->prev = list->last;
		element->next = NULL;
		list->last = element;
	}
	else {
		// active <-> after_inserted   {inserted}
		list->active->next->prev = element; // inserted <- after_inserted
		element->next = list->active->next; // inserted -> after_inserted
		element->prev = list->active; // active <- inserted
		list->active->next = element; // active -> inserted
	}
	return true;
}
bool DLLstr_InsertBefore(DLLstr_T* list, char* s) {
	if (list->active == NULL) { // neaktívny zoznam
		return false;
	}
	DLLstr_el_ptr element = malloc(sizeof(struct DLLstr_element)); // nový vkladaný prvok
	if (element == NULL) {
		return false;
	}
	TRY_DEEPCOPY_STRING(element->string, s, free(element));
	if (list->first == list->active) { // aktívny prvok je prvý v zozname
		list->active->prev = element;
		element->prev = NULL;
		element->next = list->active;
		list->first = element;
	}
	else {
		// before_inserted <-> active   {inserted}
		list->active->prev->next = element; // before_inserted -> inserted
		element->next = list->active; // inserted -> active
		element->prev = list->active->prev; // before_inserted <- inserted
		list->active->prev = element; // inserted <- active 
	}
	return true;
}

void DLLstr_DeleteFirst(DLLstr_T* list) {
	if (list->first != NULL) { // zoznam nie je prázdny
		if (list->first == list->active) { // prvý prvok je aj aktívny
			list->active = NULL;
		}
		if (list->first == list->last) { // zoznam obsahuje len 1 prvok
			// treba prepísať ukazateľ na prvý aj posledný prvok na NULL
			list->last = NULL;
		}
		DLLstr_el_ptr deleted = list->first; // uvoľnovaný prvok
		list->first = list->first->next;
		DLLstr_ElementDestroy(deleted);
		// nezabudnúť vymazať v druhom prvku ukazateľ na mazaný prvý prvok
		if (list->first != NULL) {
			list->first->prev = NULL;
		}
	}
}

void DLLstr_DeleteLast(DLLstr_T* list) {
	if (list->last != NULL) { // zoznam nie je prázdny
		if (list->last == list->active) { // posledný prvok je aj aktívny
			list->active = NULL;
		}
		if (list->first == list->last) { // zoznam obsahuje len 1 prvok
			// treba prepísať ukazateľ na prvý aj posledný prvok na NULL
			list->first = NULL;
		}
		DLLstr_el_ptr deleted = list->last; // uvoľnovaný prvok
		list->last = list->last->prev;
		DLLstr_ElementDestroy(deleted);
		// nezabudnúť vymazať v predposlednom prvku ukazateľ na mazaný posledný prvok
		if (list->last != NULL) {
			list->last->next = NULL;
		}
	}
}
void DLLstr_DeleteAfter(DLLstr_T* list) {
	if (list->active == NULL) { // neaktívny zoznam
		return;
	}
	if (list->active == list->last) { // aktívny prvok je posledný prvok zoznamu
		return;
	}
	if (list->active->next == list->last) { // aktívny prvok je predposledný
		// aktívny prvok sa teda stáva posledným
		list->last = list->active;
		DLLstr_ElementDestroy(list->active->next);
		list->active->next = NULL;
	}
	else { // za aktívnym prvkom sú aspoň 2 prvky (vrátane rušeného)
		// active <-> deleted <-> new_next_active
		DLLstr_el_ptr deleted = list->active->next;
		list->active->next = deleted->next; // active -> new_next_active
		deleted->next->prev = list->active; // active <- new_next_active
		// active <-> new_next_active
		DLLstr_ElementDestroy(deleted);
	}
}

void DLLstr_DeleteBefore(DLLstr_T* list) {
	if (list->active == NULL) { // neaktívny zoznam
		return;
	}
	if (list->active == list->first) { // aktívny prvok je prvý
		return;
	}
	if (list->active->prev == list->first) { // aktívny prvok je druhý
		// aktívny prvok sa teda stáva prvým
		list->first = list->active;
		DLLstr_ElementDestroy(list->active->prev);
		list->active->prev = NULL;
	}
	else { // pred aktívnym prvkom sú aspoň 2 prvky (vrátane rušeného)
		// new_previous_active <-> deleted <-> active
		DLLstr_el_ptr deleted = list->active->prev;
		list->active->prev = deleted->prev; // new_previous_active <- active
		deleted->prev->next = list->active; // new_previous_active -> active
		// new_next_active <-> active
		DLLstr_ElementDestroy(deleted);
	}
}

void DLLstr_Dispose(DLLstr_T* list) {
	DLLstr_el_ptr element = list->first;
	DLLstr_el_ptr previous;
	while (element != NULL) {
		previous = element;
		element = element->next; // posuniem sa ďalej
		DLLstr_ElementDestroy(previous);
	}
	list->first = NULL;
	list->active = NULL;
	list->last = NULL;
}

void DLLstr_printContent(DLLstr_T *list) {
	str_T text;
	StrInit(&text);
	DLLstr_First(list);
	while (DLLstr_IsActive(list))
	{
		DLLstr_GetValue(list, &text);
		printf("%s\n", StrRead(&text));
		DLLstr_Next(list);
	}
	StrDestroy(&text);
}

/* Koniec súboru dll.c */
