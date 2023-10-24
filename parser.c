/** Projekt IFJ2023
 * @file parser.c
 * @brief Syntaktický a sémantický anayzátor
 * @author 
 * @date 
 */

#include "parser.h"

token_T *tkn;

SymTab_T *symt;

/**
 * @brief Indikuje, či sa parser nachádza vo vnútri cykla.
*/
static bool parser_inside_loop = false;

/**
 * @brief Meno náveštia na najvrchnejší cyklus
*/
// StrR first_loop_label;

/**
 * 
*/
// DLL variables_declared_inside_loop

/**
 * @brief Uvoľní aktuálne načítaný token v globálnej premennej tkn a nahradí ho novým zo scannera
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int nextToken() {
    destroyToken(tkn);
    tkn = getToken();
    if (tkn == NULL) return COMPILER_ERROR;
    if (tkn->type == INVALID) return LEX_ERR;
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie dátového typu
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token LET alebo VAR
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseDataType(TSData_T *var_info) {
    // <TYPE>  ->  {Int, Double, String} <QUESTMARK>
    int data_type;
    switch (tkn->type)
    {
    case INT_TYPE:
    case DOUBLE_TYPE:
    case STRING_TYPE:
        data_type = tkn->type;
        //var_info->type = ...;
        break;
    default:
        return SYN_ERR;
        break;
    }
    int tkn_err = nextToken();
    if( tkn_err != 0) return tkn_err;
    if(tkn->type == QUEST_MARK) {
        switch (data_type)
        {
        case INT_TYPE:
            var_info->type = SYM_TYPE_INT_NIL;
            break;
        // TO DO
        default:
            break;
        }
    }
    else {
        storeToken(tkn);
    }
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie deklarácie/definície premennej
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token LET alebo VAR
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int variableDecl() {
    // <STAT>  ->  {let,var} id <DEF_VAR>
    bool let = tkn->type == LET ? true : false;
    int comp_err = nextToken();
    if( comp_err != 0) return comp_err;
    if ( tkn->type != ID ) {
        return SYN_ERR;
    }
    // --- vytvorenie noveho symbolu v tabulke symbolov
    if( (comp_err=nextToken()) != 0) return comp_err;
    switch (tkn->type)
    {
    case COLON:
        if( (comp_err=nextToken()) != 0) return comp_err;
        //if( (comp_err=parseDataType()) != 0) return comp_err;
        // defineVar()
        break;
    
    default:
        break;
    }
}

/**
 * @brief Pokračujúce pravidlo pre spracovanie deklarácie/definície premennej
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token LET alebo VAR
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int defineVar(TSData_T *var_info) {
    // <DEF_VAR>   ->  : <TYPE> <INIT_VAL>
}

int parse() {
    switch (tkn->type)
    {
    case LET:
    case VAR:
        
        /* code */
        break;
    
    default:
        break;
    }
}


/* Koniec súboru parser.c */
