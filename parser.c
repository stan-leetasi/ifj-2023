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
 * @brief Precedenčná syntaktická analýza výrazov
 * @details Táto funkcia očakáva globálnu premennú tkn prázdnu
 * @param result_type Dátový typ výsledku
 * @param first_token Prvý token
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseExpression(char *result_type, token_T *first_token){
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
 * @brief Pravidlo pre spracovanie priradenia
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token '='
 * @param result_type Dátový typ výsledku
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int assignment(char *result_type) {
    int comp_err = nextToken();
    if( comp_err != 0) return comp_err;
    token_T *first_tkn;
    if( tkn->type == BRT_RND_L ){
        // <ASSIGN>  ->  exp
        if( (comp_err=parseExpression(result_type)) != 0) return comp_err;
    }
    else if( tkn->type == ID ){
        if( (comp_err=nextToken()) != 0) return comp_err;
        if(tkn->type == BRT_RND_L) {
            // <ASSIGN>  ->  <CALL_FN>
        }
        else {
            
        }
    }
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
        // <DEF_VAR>  ->  : <TYPE> <INIT_VAL>
        if( (comp_err=nextToken()) != 0) return comp_err;
        //if( (comp_err=parseDataType()) != 0) return comp_err;
        if( (comp_err=nextToken()) != 0) return comp_err;
        if (tkn->type == ASSIGN) {
            // <INIT_VAL>  ->  = <ASSIGN>
            if( (comp_err=nextToken()) != 0) return comp_err;


        }
        else {
            // <INIT_VAL>  ->  €
            storeToken(tkn);
            return COMPILATION_OK;
        }
        break;
    case ASSIGN:
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
