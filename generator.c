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

void createInstructionString(str_T *, int, char *,...);

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
    return COMPILATION_OK;
}

int genDefVarsBeforeLoop(char *label, DLLstr_T *variables) {
    str_T string;
    str_T var;
    StrInit(&string);
    StrInit(&var);
    DLLstr_T *list;
    
    if (parser_inside_fn_def) {
        list = &code_fn;
    } else {
        list = &code_main;
    }

    DLLstr_First(variables);
    while(DLLstr_IsActive(variables)) {
        DLLstr_GetValue(variables, &var);
        createInstructionString(&string, var.size, "DEFVAR %s", StrRead(&var));
        DLLstr_InsertLast(list, StrRead(&string) );
        DLLstr_Next(variables);
    }
    DLLstr_GetValue(variables, &var);
    createInstructionString(&string, strlen(label), "LABEL %s", label);
    DLLstr_InsertLast(list, StrRead(&string));

    StrDestroy(&string);
    StrDestroy(&var);
    return COMPILATION_OK;
}

int genFnDefBegin(char *fn, DLLstr_T *params) {
    //Výsledný řetězec, kde bude zapsaná instrukce
    str_T string;
    //zde budou uloženy parametry funkce
    str_T par;
    //Inicializace řetězců
    StrInit(&string);
    StrInit(&par);

    DLLstr_InsertLast(&code_fn, "CREATEFRAME");
    DLLstr_InsertLast(&code_fn, "PUSHFRAME");
    DLLstr_First(params);
    while(DLLstr_IsActive(params)) {
        DLLstr_GetValue(params, &par);

        createInstructionString(&string, par.size, "DEFVAR LF@%s%%", StrRead(&par));
        DLLstr_InsertLast(&code_fn, StrRead(&string));
        createInstructionString(&string, par.size, "POPS LF@%s%%", StrRead(&par));
        DLLstr_InsertLast(&code_fn, StrRead(&string));

        DLLstr_Next(params);
    }
    StrDestroy(&string);
    StrDestroy(&par);
    return COMPILATION_OK;
}

int genFnCall(char *fn, DLLstr_T *args) {
    //Výsledný řetězec, kde bude zapsaná instrukce
    str_T string;
    //zde budou uloženy argumenty funkce fn
    str_T arg;
    //Inicializace řetězců
    StrInit(&string);
    StrInit(&arg);

    DLLstr_T *list;
    //Zde se rozhodne, do kterého seznamu se vloží instrukce
    if (parser_inside_fn_def) {
        list = &code_fn;
    } else {
        list = &code_main;
    }
    DLLstr_Last(args);
    //Průchod přes všechny argumenty funkce
    while (DLLstr_IsActive(args)) {
        DLLstr_GetValue(args, &arg);
        StrFillWith(&string, "PUSH ");
        //Nyní bude v řetězci "string" uložena celá instrukce
        StrCat(&string, &arg);
        //Výběr, do kterého seznamu se uloží instrukce
        DLLstr_InsertLast(list, StrRead(&string));
        DLLstr_Previous(args);
    }
    //Vložení na zásobník CALL instrukce
    StrFillWith(&string, "CALL ");
    StrFillWith(&arg, fn);
    StrCat(&string, &arg);
    DLLstr_InsertLast(list, StrRead(&string));
    //Uvolnění řetězců
    StrDestroy(&string);
    StrDestroy(&arg);
    return COMPILATION_OK;
}


/**
 *  Pomocná funkce, která vytvoří řetězec instrukce a uloží jej do "str"
 *  "str" = uložený řetězec
 *  "len_of_longest_arg" = délka nejdelšího argumentu formátu
 *  "format" = formátovaný řetězec
 */
void createInstructionString(str_T *str, int len_of_longest_arg, char *format,...) {
    va_list args;
    char buffer[strlen(format) + len_of_longest_arg + 1];
    va_start(args, format);
    snprintf(buffer, sizeof(buffer), format, va_arg(args, char *));
    StrFillWith(str, buffer);
    va_end(args);
}


/* Koniec súboru generator.c */
