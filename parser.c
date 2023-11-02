/** Projekt IFJ2023
 * @file parser.c
 * @brief Syntaktický a sémantický anayzátor
 * @author Michal Krulich (xkruli03)
 * @date 25.10.2023
 */

#include "parser.h"

token_T* tkn;

SymTab_T symt;

DLLstr_T code_fn;
DLLstr_T code_main;

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
static DLLstr_T* variables_declared_inside_loop;

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
int parseExpression(char* result_type, token_T* first_token) {
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie dátového typu, zapíše do var_info->type spracovaný typ
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token COLON
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseDataType(TSData_T* var_info) {
    // <TYPE>  ->  {Int, Double, String} <QUESTMARK>
    switch (tkn->type)
    {
    case INT_TYPE:
        var_info->type = SYM_TYPE_INT;
        break;
    case DOUBLE_TYPE:
        var_info->type = SYM_TYPE_DOUBLE;
        break;
    case STRING_TYPE:
        var_info->type = SYM_TYPE_STRING;
        break;
    default:
        return SYN_ERR;
        break;
    }
    TRY_OR_EXIT(nextToken());
    if (tkn->type == QUEST_MARK) {
        switch (var_info->type)
        {
        case SYM_TYPE_INT:
            var_info->type = SYM_TYPE_INT_NIL;
            break;
        case SYM_TYPE_DOUBLE:
            var_info->type = SYM_TYPE_DOUBLE_NIL;
            break;
        case SYM_TYPE_STRING:
            var_info->type = SYM_TYPE_STRING_NIL;
            break;
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
 * @brief Pravidlo pre spracovanie termu
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token
 * @param term_type Dátový typ termu
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseTerm(char* term_type) {
    switch (tkn->type)
    {
    case ID:
        TSData_T* variable = SymTabLookup(&symt, StrRead(&(tkn->atr)));
        if (variable == NULL) return SEM_ERR_UNDEF;
        if (variable->type == SYM_TYPE_FUNC) return SEM_ERR_RETURN;
        if (!(variable->init)) return SEM_ERR_UNDEF;
        *term_type = variable->type;
        break;
    case INT_CONST:
        *term_type = SYM_TYPE_INT;
        break;
    case DOUBLE_CONST:
        *term_type = SYM_TYPE_DOUBLE;
        break;
    case STRING_CONST:
        *term_type = SYM_TYPE_STRING;
        break;
    case NIL:
        *term_type = SYM_TYPE_NIL;
        break;
    default:
        return SYN_ERR;
        break;
    }
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie priradenia
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token '='
 * @param result_type Dátový typ výsledku
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseAssignment(char* result_type) {
    TRY_OR_EXIT(nextToken());
    token_T* first_tkn;
    switch (tkn->type)
    {
    case BRT_RND_L:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
    case NIL:
        // parseExpression()
        break;
    case ID:
        first_tkn = tkn;
        tkn = getToken();
        if (tkn == NULL) return COMPILER_ERROR;
        if (tkn->type == INVALID) return LEX_ERR;
        if (tkn->type == BRT_RND_L) {
            // <ASSIGN>  ->  <CALL_FN>
        }
        else {
            storeToken(tkn);
            tkn = first_tkn;
            // parseExpression
        }
        break;
    default:
        return SYN_ERR;
        break;
    }
    // *result_type = ...;
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie deklarácie/definície premennej
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token LET alebo VAR
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseVariableDecl() {
    // <STAT>  ->  {let,var} id <DEF_VAR>
    bool let = tkn->type == LET ? true : false;
    TRY_OR_EXIT(nextToken());
    if (tkn->type != ID) {
        return SYN_ERR;
    }
    // --- vytvorenie noveho symbolu v tabulke symbolov
    if (SymTabLookupLocal(&symt, StrRead(&(tkn->atr))) != NULL) {
        return SEM_ERR_REDEF;
    }
    TSData_T* variable = SymTabCreateElement(StrRead(&(tkn->atr)));
    if (variable == NULL)
    {
        return COMPILER_ERROR;
    }
    variable->init = false;
    variable->let = let;
    variable->sig = NULL;
    variable->type = SYM_TYPE_UNKNOWN;

    if (SymTabInsertLocal(&symt, variable)) {
        free(variable);
        return COMPILER_ERROR;
    }

    TRY_OR_EXIT(nextToken());
    switch (tkn->type)
    {
    case COLON:
        // <DEF_VAR>  ->  : <TYPE> <INIT_VAL>
        TRY_OR_EXIT(nextToken());
        TRY_OR_EXIT(parseDataType(variable));
        TRY_OR_EXIT(nextToken());
        if (tkn->type == ASSIGN) {
            // <INIT_VAL>  ->  = <ASSIGN>
            char assign_type = SYM_TYPE_UNKNOWN;
            TRY_OR_EXIT(parseAssignment(&assign_type));
            if (assign_type != variable->type) { // unknown TODO
                return SEM_ERR_TYPE;
            }
        }
        else {
            // <INIT_VAL>  ->  €
            storeToken(tkn);
        }
        return COMPILATION_OK;
        break;
    case ASSIGN:
        // <DEF_VAR>   ->  = <ASSIGN>
        TRY_OR_EXIT(parseAssignment(&(variable->type)));
        // unknown TODO
        return COMPILATION_OK;
        break;
    default:
        return SYN_ERR;
        break;
    }
    return SYN_ERR;
}

/**
 * @brief Pravidlo pre spracovanie definície parametrov funkcie, končí načítaním pravej zátvorky
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token FUNC
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFunctionSignature() {
    // <FN_SIG>        ->  <FN_PAR> <FN_PAR_NEXT>
    // <FN_SIG>        ->  €
    TRY_OR_EXIT(nextToken());

    size_t params = 0;

    while (tkn->type != BRT_RND_R)
    {
        // <FN_PAR_NEXT>   ->  , <FN_PAR> <FN_PAR_NEXT>
        // <FN_PAR_NEXT>   ->  €
        // <FN_PAR>        ->  id id : <TYPE>
        // <FN_PAR>        ->  _  id : <TYPE>
        if (params > 0) {
            if (tkn->type != COMMA) return SYN_ERR;
            TRY_OR_EXIT(nextToken());
        }
        if (!(tkn->type == ID && tkn->type == UNDERSCORE)) return SYN_ERR;
        TRY_OR_EXIT(nextToken());
        if (tkn->type != ID) return SYN_ERR;
        TRY_OR_EXIT(nextToken());
        if (tkn->type != COLON) return SYN_ERR;
        TSData_T data_el; // TODO zmenit
        TRY_OR_EXIT(parseDataType(&data_el));
        params++;
    }

    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie definície funkcie
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token FUNC
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFunction() {
    // <STAT> ->  func id ( <FN_SIG> ) <FN_RET_TYPE> { <PROG> }
    bool code_inside_fn_def = parser_inside_fn_def;
    parser_inside_fn_def = true;

    TRY_OR_EXIT(nextToken());
    if (tkn->type != ID) return SYN_ERR;

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) return SYN_ERR;

    TRY_OR_EXIT(parseFunctionSignature());

    TRY_OR_EXIT(nextToken);

    if (tkn->type == ARROW) {
        //<FN_RET_TYPE>   ->  "->" <TYPE>
        TSData_T data_el; // TODO zmenit
        TRY_OR_EXIT(parseDataType(&data_el));
        TRY_OR_EXIT(nextToken());
    }
    else if (tkn->type == BRT_CUR_L)
    {
        // <FN_RET_TYPE>   ->  €
    }
    else {
        return SYN_ERR;
    }

    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    parser_inside_fn_def = code_inside_fn_def;
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie podmieneného bloku kódu
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token IF
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseIf() {
    // <STAT>  ->  if <COND> { <PROG> } else { <PROG> }
    TRY_OR_EXIT(nextToken());
    switch (tkn->type)
    {
    case LET:
        //  <COND> ->  let id
        TRY_OR_EXIT(nextToken());
        if (tkn->type != ID) return SYN_ERR;
        // TODO
        break;
    case ID:
    case BRT_RND_L:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
        // <COND> ->  exp
        token_T* t = tkn;
        tkn = NULL;
        char exp_type;
        parseExpression(&exp_type, t);
        break;
    default:
        return SYN_ERR;
        break;
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_CUR_L) return SYN_ERR;
    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != ELSE) return SYN_ERR;


    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_CUR_L) return SYN_ERR;
    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie cyklu while
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token WHILE
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseWhile() {
    // <STAT>      ->  while exp { <PROG> }
    bool loop_inside_loop = parser_inside_loop;
    parser_inside_loop = true;

    TRY_OR_EXIT(nextToken());
    // TODO pridat konstanty INT_CONT, ... do if
    if (!(tkn->type == ID || tkn->type == BRT_RND_L)) return SYN_ERR;

    token_T* t = tkn;
    tkn = NULL;
    char exp_type;
    parseExpression(&exp_type, t);
    // TODO

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_CUR_L) return SYN_ERR;
    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    parser_inside_loop = loop_inside_loop;
    return COMPILATION_OK;
}

/**
 * @brief Pokračujúce pravidlo pre spracovanie deklarácie/definície premennej
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token LET alebo VAR
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int defineVar(TSData_T* var_info) {
    // <DEF_VAR>   ->  : <TYPE> <INIT_VAL>
}

/* --- FUNKCIE DEKLAROVANÉ V PARSER.H --- */

bool initializeParser() {
    if (!SymTabInit(&symt)) return false;
    DLLstr_Init(&code_fn);
    DLLstr_Init(&code_main);
    return true;
}

int parse() {
    switch (tkn->type)
    {
    case LET:
    case VAR:
        TRY_OR_EXIT(parseVariableDecl());
        break;
    case ID:
        // <STAT>   ->  <CALL_FN>
        // <STAT>   ->  id = <ASSIGN>
        // TODO podobné parseAssignment => refaktorizacia ???
        token_T* first_tkn = tkn;
        tkn = getToken();
        if (tkn == NULL) return COMPILER_ERROR;
        if (tkn->type == INVALID) return LEX_ERR;
        if (tkn->type == BRT_RND_L) {
            // <STAT>   ->  <CALL_FN>
        }
        else if (tkn->type == ASSIGN) {
            // <STAT>   ->  id = <ASSIGN>
            storeToken(tkn);
            tkn = first_tkn;
            // parseAssignment
        }
        else {
            return LEX_ERR;
        }
        break;
    case BRT_CUR_L:
        // <STAT>  ->  { <PROG> }
        TRY_OR_EXIT(nextToken());
        //SymTabAddLocalBlock(&symt);
        while (tkn->type != BRT_CUR_R) {
            TRY_OR_EXIT(parse());
            TRY_OR_EXIT(nextToken());
        }
        break;
    case FUNC:
        // <STAT> ->  func id ( <FN_SIG> ) <FN_RET_TYPE> { <PROG> }
        TRY_OR_EXIT(parseFunction());
        break;
    case RETURN:
        // <STAT>      ->  return <RET_VAL>
        TRY_OR_EXIT(nextToken());
        char term_type;
        TRY_OR_EXIT(parseTerm(&term_type));
        break;
    case IF:
        // <STAT>      ->  if <COND> { <PROG> } else { <PROG> }
        TRY_OR_EXIT(parseIf());
        break;
    case WHILE:
        // <STAT>      ->  while exp { <PROG> }
        TRY_OR_EXIT(parseWhile());
        break;
    default:
        return LEX_ERR;
        break;
    }

    return COMPILATION_OK;
}


/* Koniec súboru parser.c */
