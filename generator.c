/** Projekt IFJ2023
 * @file generator.c
 * @brief Generátor cieľového kódu
 * @author 
 * @date 11.11.2023
 */

#include "generator.h"
#include <stdarg.h>

DLLstr_T code_fn;
DLLstr_T code_main;

void fnParamIdentificator(char *identificator, str_T *id);

int genUniqVar(char *scope, char *sub, str_T *id) {
    return COMPILATION_OK;
}

int genUniqLabel(char *fn, char *sub, str_T *label){
    return COMPILATION_OK;
}

int genConstVal(int const_type, char *value, str_T *cval) {
    return COMPILATION_OK;
}

int genCode(char *instruction, char *op1, char *op2, char *op3) {
    if (instruction == NULL)
        return COMPILER_ERROR;

    str_T *code;
    StrInit(code);

    StrFillWith(code, instruction);

    if (op1 != NULL) {
        StrAppend(code, ' ');
        StrCatString(code, op1);
    }

    if (op2 != NULL) {
        StrAppend(code, ' ');
        StrCatString(code, op2);
    }

    if (op3 != NULL) {
        StrAppend(code, ' ');
        StrCatString(code, op3);
    }

    bool insert;

    if(parser_inside_fn_def == true)
        insert = DLLstr_InsertLast(&code_fn, StrRead(code));
    else
        insert = DLLstr_InsertLast(&code_main, StrRead(code));

    return COMPILATION_OK;
}

int genDefVarsBeforeLoop(char *label, DLLstr_T *variables) {
    str_T var;
    StrInit(&var);

    DLLstr_First(variables);
    while(DLLstr_IsActive(variables)) {
        DLLstr_GetValue(variables, &var);

        genCode("DEFVAR",StrRead(&var), NULL, NULL);
        DLLstr_Next(variables);
    }

    genCode("LABEL", label, NULL, NULL);
    StrDestroy(&var);

    return COMPILATION_OK;
}

int genFnDefBegin(char *fn, DLLstr_T *params) {
    //zde bude zapsán celý identifikator parametru
    str_T idpar;
    //zde budou uloženy parametry funkce
    str_T fnpar;
    //Inicializace řetězců
    StrInit(&idpar);
    StrInit(&fnpar);

    genCode("LABEL", fn, NULL, NULL);
    genCode("CREATEFRAME", NULL, NULL, NULL);
    genCode("PUSHFRAME", NULL, NULL, NULL);
    DLLstr_First(params);

    while(DLLstr_IsActive(params)) {
        DLLstr_GetValue(params, &fnpar);
        fnParamIdentificator(StrRead(&fnpar), &idpar);
        genCode("DEFVAR", StrRead(&idpar), NULL, NULL);
        genCode("POPS", StrRead(&idpar), NULL, NULL);
        DLLstr_Next(params);
    }

    StrDestroy(&idpar);
    StrDestroy(&fnpar);
    return COMPILATION_OK;
}

int genFnCall(char *fn, DLLstr_T *args) {
    //zde budou uloženy argumenty funkce fn
    str_T arg;
    StrInit(&arg);

    DLLstr_Last(args);
    //Průchod přes všechny argumenty funkce
    while (DLLstr_IsActive(args)) {
        DLLstr_GetValue(args, &arg);
        genCode("PUSH", StrRead(&arg), NULL, NULL);
        DLLstr_Previous(args);
    }
    //Vložení na zásobník CALL instrukce
    genCode("CALL", fn, NULL, NULL);
   
    //Uvolnění řetězců
    StrDestroy(&arg);
    return COMPILATION_OK;
}


/**
 *  Pomocná funkce, která vytvoří řetězec instrukce a uloží jej do "str"
 *  "str" = uložený řetězec
 *  "len_of_longest_arg" = délka nejdelšího argumentu formátu
 *  "format" = formátovaný řetězec
 */
void fnParamIdentificator(char *identificator, str_T *id) {
    StrFillWith(id, "LF@");
    StrCatString(id, identificator);
    StrAppend(id, '%');
}


/* Koniec súboru generator.c */