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
#include "decode.h"

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
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn.
 * 
 * Vygenerovaný kód:    náveštie na funkciu, vytvorí nový rámec, vloží ho do zásobníka,
 *                      definuje v novom rámci premenné parametrov funkcie,
 *                      zapíše do nich hodnoty zo zásobníka (na vrchole je prvý argument).
 * 
 * Identifikátory parametrov vyzerajú nasledovne "LF@<id>%".
 * 
 * Príklad:
 *      genFnDefBegin("sum", {"a", "b"}),
 *      vygeneruje kód:
 * 
 *      ...
 *      LABEL sum
 *      CREATEFRAME
 *      PUSHFRAME
 *      DEFVAR  LF@a%
 *      POPS    LF@a%
 *      DEFVAR  LF@b%
 *      POPS    LF@b%
 * 
 * @brief Vygeneruje kód začiatku definície funkcie, resp. príprava nového rámca a argumentov.
 * @param fn Názov funkcie
 * @param params Identifikátory parametrov funkcie
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genFnDefBegin(char *fn, DLLstr_T *params);

/**
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn pokiaľ parser_inside_fn_def==true
 * (globálna premenná v parser.h), inak na koniec code_main.
 * 
 * Vygenerovaný kód:    vloží do zásobníka argumenty (prvý argument bude na vrchole, posledný na dne)
 *                       a predá riadenie pomocou CALL.
 * 
 * Príklad:
 *      genFnCall("sum", {"int@6", "LF@x$1"}),
 *      vygeneruje kód:
 * 
 *      ...
 *      PUSH    LF@x$1
 *      PUSH    int@6
 *      CALL sum
 * 
 * @brief Vygeneruje kód volania funkcie.
 * @param fn Názov funkcie
 * @param args Predávané argumenty
 * @return V prípade úspechu COMPILATION_OK, inak COMPILER_ERROR.
*/
int genFnCall(char *fn, DLLstr_T *args);

/**
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn pokiaľ parser_inside_fn_def==true
 * (globálna premenná v parser.h), inak na koniec code_main.
 * 
 * Argumenty funkce se budou zpracovávat z leva doprava. Jako kod se vygeneruje sekvence WRITE prikazu s argumenty v args
 * Ve tvaru:
 * 
 *      ...
 *      WRITE args[0]
 *      WRITE args[1]
 *      WRITE args[3]
 *      ...      ...
 * 
 * @brief Vygeneruje kód potřebný pro provedení build-in funkce write()
 * 
 * @param args Předané argumenty
 * @return V případě úspěchu COMPILATION_OK, v opačném případě COMPILER_ERROR
 */
int genWrite(DLLstr_T *args);

/**
 * Vygenerovaný kód bude vložený na koniec zoznamu code_fn.
 * 
 * Vygeneruje se následující kód:
 *      LABEL substring
        CREATEFRAME
        PUSHFRAME

        DEFVAR LF@?!end$1
        POPS LF@?!end$1
        DEFVAR LF@?!begin$2
        POPS LF@?!begin$2
        DEFVAR LF@?!string$3
        POPS LF@?!string$3

        DEFVAR LF@?!strlen$4
        DEFVAR LF@?!check$5
        DEFVAR LF@?!output$6
        DEFVAR LF@?!char$7

        MOVE LF@?!output$6 nil@nil
        STRLEN LF@?!strlen$4 LF@?!string$3

        GT LF@?!check$5 LF@?!begin$2 LF@?!strlen$4
        EQ LF@?!check$5 LF@?!begin$2 LF@?!strlen$4
        JUMPIFEQ end&3 LF@?!check$5 bool@true

        GT LF@?!check$5 LF@?!end$1 LF@?!strlen$4
        JUMPIFEQ end&3 LF@?!check$5 bool@true

        LT LF@?!check$5 LF@?!begin$2 int@0
        JUMPIFEQ end&3 LF@?!check$5 bool@true

        LT LF@?!check$5 LF@?!end$1 int@0
        JUMPIFEQ end&3 LF@?!check$5 bool@true

        GT LF@?!check$5 LF@?!begin$2 LF@?!end$1
        JUMPIFEQ end&3 LF@?!check$5 bool@true

        MOVE LF@?!output$6 string@
        EQ LF@?!check$5 LF@?!begin$2 LF@?!end$1
        JUMPIFEQ end&3 LF@?!check$5 bool@true

        LABEL cycle&2
            GETCHAR LF@?!strlen$4 LF@?!string$3 LF@?!begin$2
            CONCAT LF@?!output$6 LF@?!output$6 LF@?!strlen$4
            ADD LF@?!begin$2 LF@?!begin$2 int@1
            JUMPIFNEQ cycle&2 LF@?!begin$2 LF@?!end$1

        LABEL end&3
            PUSHS LF@?!output$6
        RETURN
 * 
 * Všechny proměnné a labely, použité v této funkci, musí mít unikatní 
 * pojmenování v rámci celého programu
 * 
 * @brief Vygeneruje kód potřebný pro provedení build-in funkce substring
 * 
 * @param ans Předpokládá se nějaké externí proměnná (z pohledu této funkce), kam bude uložen výsledek
 * @return V případě úspěchu COMPILATION_OK, v opačném případě COMPILER_ERROR
 */
int genSubstring();

#endif // ifndef _GENERATOR_H_
/* Koniec súboru generator.h */
