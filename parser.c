/** Projekt IFJ2023
 * @file parser.c
 * @brief Syntaktický a sémantický anayzátor
 * @author Michal Krulich (xkruli03)
 * @date 25.10.2023
 */

#include "parser.h"

token_T *tkn;

SymTab_T *symt;

DLLstr_T *code_fn;
DLLstr_T *code_main;

/**
 * @brief Indikuje, či sa parser nachádza vo vnútri cykla.
*/
static bool parser_inside_loop = false;

/**
 * @brief Meno náveštia na najvrchnejší cyklus
*/
// StrR first_loop_label;

/**
 * @brief Zoznam premenných,, ktoré musia byť dekalrované pred prvým nespracovaným cyklom
*/
static DLLstr_T *variables_declared_inside_loop;

/**
 * @brief Indikuje, či sa aktuálne spracúva kód vo vnútri funkcie.
 * @details Podľa toho sa generovaný kód ukladá buď do code_fn alebo code_main.
*/
static bool parser_inside_fn_def = false;

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
 * Táto funkcia:
 *  - žiada o tokeny dokým je možné vytvoriť zmysluplný výraz.
 *  - prevádza infixový výraz na postfixový
 *  - na základe postfixového výrazu generuje cieľový kód, pričom kontroluje 
 *    sémantiku za pomoci tabuľky symbolov:
 *          - či sú premenné deklarované a inicializované
 *          - či sedia dátové typy operandov
 * 
 * @brief Precedenčná syntaktická analýza výrazov
 * @details Táto funkcia očakáva globálnu premennú tkn prázdnu a taktiež ju aj zanechá prázdnu.
 * @param result_type Dátový typ výsledku výrazu
 * @param first_token Prvý token výrazu (netreba načítavať zo skenera)
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
        // if( (comp_err=parseExpression(result_typem, tkn)) != 0) return comp_err;
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
