/** Projekt IFJ2023
 * @file generator.h
 * @brief Generátor cieľového kódu
 * @author Michal Krulich (xkruli03)
 * @date 11.11.2023
 */

#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include "dll.h"
#include "parser.h"

#define BOOL_CONST -50  ///< Konštanta - true/false ... Musí byť rôzna od hodnôt token_ids.

/**
 * @brief Vygenerovaný kód pre funkcie
*/
extern DLLstr_T code_fn;

/**
 * @brief Vygenerovaný kód pre hlavný program (všetko mimo funkcií)
*/
extern DLLstr_T code_main;

/**
 * Vygenerovaný identifikátor bude v tvare "<scope>@<id>$<cislo>", kde
 *      <scope> je poskytnutý prefix,
 *      <id> je poskytnutý podreťazec,
 *      <cislo> je unikátne číslo, ktoré bude pre každú premennú rôzne.
 * 
 * Príklady:
 *      genUniqVar("GF", "x", &vysledok) => "GF@x$1"
 *      genUniqVar("LF", "x", &vysledok) => "LF@x$2"
 *      genUniqVar("LF", "n", &vysledok) => "LF@n$3"
 *      genUniqVar("TF", "x", &vysledok) => "TF@x$4"
 * 
 * @brief Vygeneruje unikátny identifikátor pre premennú v cieľovom kóde.
 * @param scope Prefix identifikátora.
 * @param sub Podreťazec identifikátora.
 * @param id Výsledný unikátny identifikátor. V prípade neúspechu nedefinované.
 *           Dátovú štruktúru treba inicializovať.
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genUniqVar(char *scope, char *sub, str_T *id);

/**
 * Vygenerované náveštie bude v tvare "<fn>&<sub><cislo>", kde 
 *      <fn> je prefix,
 *      <sub> je podreťazec unikátneho náveštia,
 *      <cislo> je unikátne číslo, ktoré bude pre každé náveštie rôzne.
 *      
 * Príklady:
 *      genUniqLabel("fib", "while", &vysledok)     => "fib&while1"
 *      genUniqLabel("fib", "if", &vysledok)        => "fib&if2"
 *      genUniqLabel("pow", "while", &vysledok)     => "pow&while3"
 *      genUniqLabel("", "if", &vysledok)           => "&if4"
 * 
 * @brief Vygeneruje unikátny identifikátor pre náveštie v cieľovom kóde.
 * @param fn Prefix náveštia, resp. funkcia, v ktorej sa nachádza.
 * @param sub Podreťazec unikátneho náveštia.
 * @param label Výsledný unikátny názov náveštia. V prípade neúspechu nedefinované.
 *              Dátovú štruktúru treba inicializovať.
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genUniqLabel(char *fn, char *sub, str_T *label);

/**
 * V prípade reťazca treba reťazec prispoôsobiť požiadavkam cieľového kódu pomocou funkcie decode.
 *      
 * Príklady:
 *      genConstVal(INT_CONST, "53", &vysledok)         => "int@53"
 *      genConstVal(DOUBLE_CONST, "3.14", &vysledok)    => "float@0x1.91eb86p+1"
 *      genConstVal(NIL, "", &vysledok)                 => "nil@nil"
 *      genConstVal(STRING_CONST, "a b", &vysledok)     => "string@a\032b"
 *      genConstVal(BOOL_CONST, "true", &vysledok)      => "bool@true"
 * 
 * @brief Vygeneruje konštantu v cieľovom kóde.
 * @param const_type Typ konštanty. Možné hodnoty: INT_CONST, DOUBLE_CONST, STRING_CONST, BOOL_CONST a NIL.
 * @param value Hodnota konštanty.
 * @param cval Výsledná konštanta v cieľovom kóde.
 *              Dátovú štruktúru treba inicializovať.
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genConstVal(int const_type, char *value, str_T *cval);

/**
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn pokiaľ parser_inside_fn_def==true
 * (globálna premenná v parser.h), inak na koniec code_main.
 * 
 * Príklady:
 *      genCode("ADD", "GF@x", "GF@y", "LF@z") vygeneruje "ADD GF@x GF@y LF@z"
 *      genCode("PUSHFRAME", NULL, NULL, NULL) vygeneruje "PUSHFRAME"
 * 
 * @brief Vygeneruje a vloží kód s danými argumentami do zoznamu s vygenerovanými inštrukciami. 
 * @param instruction Názov inštrukcie
 * @param op1 Prvý operand. Ak sa rovná NULL, je ignorovaný.
 * @param op2 Druhý operand. Ak sa rovná NULL, je ignorovaný.
 * @param op3 Tretí operand. Ak sa rovná NULL, je ignorovaný.
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genCode(char *instruction, char *op1, char *op2, char *op3);

/**
 * Funkcia pracuje so zoznamom code_fn pokiaľ parser_inside_fn_def==true
 * (globálna premenná v parser.h), inak s code_main.
 * 
 * Vygenerované inštrukcie definícií premenných budú vložené pred náveštie zadané náveštie,
 * resp. pred inštrukciu "LABEL <label>".
 * 
 * Príklad:
 *      genDefVarsBeforeLoop("&while25", {"LF@x$1", "LF@y$2"}),
 *      budú vygenerované dve inštrukcie:
 * 
 *      ...
 *      DEFVAR LF@x$1
 *      DEFVAR LF@y$2
 *      LABEL &while25
 *      ...
 * 
 * @brief Vygeneruje kód pre deklaráciu premenných pred zadaný cyklus.
 * @param label Názov náveštia cyklu.
 * @param variables Zoznam identifikátorov premenných, ktoré treba definovať pred cyklom.
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genDefVarsBeforeLoop(char *label, DLLstr_T *variables);

/**
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn pokiaľ parser_inside_fn_def==true
 * (globálna premenná v parser.h), inak na koniec code_main.
 * 
 * Vygenerovaný kód: vytvorí nový rámec, do dočasného rámca definuje návratovú hodnotu a parametre
 * a vloží argumenty, vloží dočasný rámec do zásobníka, a zavolá funkciu.
 * 
 * Návratová hodnota má identifikátor "TF@!retval".
 * Identifikátory parametrov vyzerajú nasledovne "TF@<id>%".
 * 
 * Príklad:
 *      genFnCall("sum", {"a", "b"}, {"int@6", "LF@x$1"}),
 *      vygeneruje kód:
 * 
 *      ...
 *      CREATEFRAME
 *      DEFVAR  TF@!retval
 *      DEFVAR  TF@a%
 *      MOVE    TF@a%   int@6
 *      DEFVAR  TF@b%
 *      MOVE    TF@b%   LF@x$1
 *      PUSHFRAME
 *      CALL sum
 * 
 * @brief Vygeneruje kód volania funkcie.
 * @param fn Názov funkcie
 * @param params Identifikátory parametrov funkcie
 * @param args Predávané argumenty
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genFnCall(char *fn, DLLstr_T *params, DLLstr_T *args);

/**
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn pokiaľ parser_inside_fn_def==true
 * (globálna premenná v parser.h), inak na koniec code_main.
 * 
 * Príklad:
 *      genFnCallExit("LF@ans$25"),
 *      vygeneruje kód:
 * 
 *      ...
 *      POPFRAME
 *      MOVE    LF@ans$25   TF@!retval
 * 
 * @brief Vygeneruje kód, potrebný pre získanie návratovej hodnoty funkcie a vymazanie rámca.
 * @param var Identifikátor premennej, do ktorej má byť uložená návratová hodnota funkcie.
 *            V prípade NULL, funkcia je void a inštrukcia MOVE nie je generovaná.
 * 
*/
int genFnCallExit(char *var);

#endif // ifndef _GENERATOR_H_
/* Koniec súboru generator.h */
