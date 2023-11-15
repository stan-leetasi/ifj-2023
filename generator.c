/** Projekt IFJ2023
 * @file generator.c
 * @brief Generátor cieľového kódu
 * @author 
 * @date 11.11.2023
 */

#include "generator.h"

DLLstr_T code_fn;
DLLstr_T code_main;

int genUniqVar(char *scope, char *sub, str_T *id) {
    static int count = 0;
    count++;

    char numStr[100];
    sprintf(numStr, "%d", count);

    if(numStr == NULL || scope == NULL || sub == NULL || id == NULL)
        return COMPILER_ERROR;

    StrCatString(id, scope);
    StrAppend(id,'@');
    StrCatString(id,sub);
    StrAppend(id,'$');
    StrCatString(id,numStr);

    // printf("id: %s\n", StrRead(id));

    return COMPILATION_OK;
}

int genUniqLabel(char *fn, char *sub, str_T *label){
    static int count = 0;
    count++;

    char numStr[100];
    sprintf(numStr, "%d", count);

    if(numStr == NULL || fn == NULL || sub == NULL)
        return COMPILER_ERROR;

    StrCatString(label, fn);
    StrAppend(label, '&');
    StrCatString(label, sub);
    StrCatString(label, numStr);


    return COMPILATION_OK;
}

int genConstVal(int const_type, char *value, str_T *cval) {  

    switch (const_type) {
        case 8:
            StrCatString(cval, "int@");
            StrCatString(cval, value);
            break;
        case 9: // float
            StrCatString(cval, "float@");
            char num[100];
            float result2 = strtof(value, NULL);
            sprintf(num, "%a", result2);
            StrCatString(cval, num);
            break;
        case 18:
            StrCatString(cval, "nil@nil");
            break;
        case 10:
            str_T decoded_value = strEncode(value);
            StrCatString(cval, StrRead(&decoded_value));            
            StrDestroy(&decoded_value);
            break;
        case -50:
            if (strcmp(value, "true") == 0) {
                StrFillWith(cval, "bool@true");
            } else {
                StrFillWith(cval, "bool@false");
            }
            break;
        default:
            return COMPILER_ERROR;
    }

    // printf("result: %s\n", StrRead(cval));

    return COMPILATION_OK;
}

int genCode(char *instruction, char *op1, char *op2, char *op3) {
    if (instruction == NULL)
        return COMPILER_ERROR;

    str_T *code;
    StrInit(code);

    StrFillWith(code, instruction);
    StrAppend(code, ' ');

    if (op1 != NULL) {
        StrCatString(code, op1);
        StrAppend(code, ' ');
    }

    if (op2 != NULL) {
        StrCatString(code, op2);
        StrAppend(code, ' ');
    }

    if (op3 != NULL) {
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

    return COMPILATION_OK;
}

int genFnDefBegin(char *fn, DLLstr_T *params) {
    return COMPILATION_OK;
}

int genFnCall(char *fn, DLLstr_T *args) {
    return COMPILATION_OK;
}

/* Koniec súboru generator.c */
