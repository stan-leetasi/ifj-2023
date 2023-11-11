/** Projekt IFJ2023
 * @file parser.c
 * @brief Syntaktický a sémantický anayzátor
 * @author Michal Krulich (xkruli03)
 * @date 11.10.2023
 */

#include "parser.h"
#include "logErr.h"
#include "exp.h"
#include "generator.h"

token_T* tkn = NULL;

SymTab_T symt;

bool parser_inside_loop = false;
str_T first_loop_label;
DLLstr_T variables_declared_inside_loop;

bool parser_inside_fn_def = false;
str_T fn_name;

/* ----------- PRIVATE FUNKCIE ----------- */

/**
 * @brief Pravidlo pre spracovanie dátového typu, zapíše do var_info->type spracovaný typ
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token COLON alebo ARROW
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseDataType(TSData_T* var_info) {
    // <TYPE>  ->  {Int, Double, String} <QUESTMARK>
    TRY_OR_EXIT(nextToken());
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
        logErrSyntax(tkn, "data type");
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
        saveToken();
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
        /* Zatiaľ len syntaktická analýza
        TSData_T* variable = SymTabLookup(&symt, StrRead(&(tkn->atr)));
        if (variable == NULL) return SEM_ERR_UNDEF;
        if (variable->type == SYM_TYPE_FUNC) return SEM_ERR_RETURN;
        if (!(variable->init)) return SEM_ERR_UNDEF;
        *term_type = variable->type;
        */
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
        logErrSyntax(tkn, "term");
        return SYN_ERR;
        break;
    }
    return COMPILATION_OK;
}


/**
 * @brief Pravidlo pre spracovanie argumentov volanej funkcie
 * @details Očakáva, že v globálnej premennej tkn je už načítaný '(', zanechá načítaný ')'
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnCallArgs() {
    // <PAR_LIST>      ->  <PAR_IN> <PAR_IN_NEXT>
    // <PAR_LIST>      ->  €
    // <PAR_IN_NEXT>   ->  , <PAR_IN> <PAR_IN_NEXT>
    // <PAR_IN_NEXT>   ->  €
    // <PAR_IN>        ->  id : term
    // <PAR_IN>        ->  term

    size_t args = 0;

    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_RND_R)
    {
        if(args > 0) {
            if(tkn->type != COMMA){
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }
        switch (tkn->type)
        {
        case ID:
            TRY_OR_EXIT(nextToken());
            if(tkn->type == COMMA) {
                // <PAR_IN> ->  term
                saveToken();
            }
            else if (tkn->type == COLON) {
                // <PAR_IN>        ->  id : term
                TRY_OR_EXIT(nextToken());
                char result_type;
                TRY_OR_EXIT(parseTerm(&result_type));
            }
            else if (tkn->type == BRT_RND_R) {
                saveToken();
            }
            else {
                logErrSyntax(tkn, "',' or ':'");
                return SYN_ERR;
            }
            break;
        case INT_CONST:
        case DOUBLE_CONST:
        case STRING_CONST:
        case NIL:
            // <PAR_IN> ->  term
            break;
        default:
            logErrSyntax(tkn, "parameter identifier or term");
            return SYN_ERR;
        }
        TRY_OR_EXIT(nextToken());
        args++;
    }

    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie volania funkcie
 * @details Očakáva, že v globálnej premennej tkn je už načítaný identifikátor volanej funkcie
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnCall(char *result_type) {
    // <CALL_FN>       ->  id ( <PAR_LIST> )
    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) {
        logErrSyntax(tkn, "'('");
        return SYN_ERR;
    }

    TRY_OR_EXIT(parseFnCallArgs());

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
        // <ASSIGN> ->  exp
        parseExpression(result_type);
        break;
    case ID:
        first_tkn = tkn;
        tkn = NULL;
        TRY_OR_EXIT(nextToken());
        if (tkn->type == BRT_RND_L) {
            // <ASSIGN>  ->  <CALL_FN>
            saveToken();
            tkn = first_tkn;
            TRY_OR_EXIT(parseFnCall(result_type));
        }
        else {
            saveToken();
            tkn = first_tkn;
            parseExpression(result_type);
        }
        break;
    default:
        logErrSyntax(tkn, "assignment or function call");
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
        logErrSyntax(tkn, "identifier");
        return SYN_ERR;
    }
    // --- vytvorenie noveho symbolu v tabulke symbolov
    if (SymTabLookupLocal(&symt, StrRead(&(tkn->atr))) != NULL
        && false /* zatiaľ pre potreby debugu syntaktickej analýzy */) {
        return SEM_ERR_REDEF;
    }
    TSData_T* variable = SymTabCreateElement(StrRead(&(tkn->atr)));
    if (variable == NULL)
    {
        logErrCompilerMemAlloc();
        return COMPILER_ERROR;
    }
    variable->init = false;
    variable->let = let;
    variable->sig = NULL;
    variable->type = SYM_TYPE_UNKNOWN;

    if (!SymTabInsertLocal(&symt, variable)) {
        free(variable);
        return COMPILER_ERROR;
    }

    TRY_OR_EXIT(nextToken());
    switch (tkn->type)
    {
    case COLON:
        // <DEF_VAR>  ->  : <TYPE> <INIT_VAL>
        TRY_OR_EXIT(parseDataType(variable));
        TRY_OR_EXIT(nextToken());
        if (tkn->type == ASSIGN) {
            // <INIT_VAL>  ->  = <ASSIGN>
            char assign_type = SYM_TYPE_UNKNOWN;
            TRY_OR_EXIT(parseAssignment(&assign_type));
            /*if (assign_type != variable->type) { // unknown TODO
                return SEM_ERR_TYPE;
            }*/
        }
        else {
            // <INIT_VAL>  ->  €
            saveToken();
        }
        break;
    case ASSIGN:
        // <DEF_VAR>   ->  = <ASSIGN>
        TRY_OR_EXIT(parseAssignment(&(variable->type)));
        // unknown TODO
        break;
    default:
        logErrSyntax(tkn, "':' or '='");
        return SYN_ERR;
        break;
    }
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie definície parametrov funkcie, končí načítaním pravej zátvorky
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token BRT_RND_L, zanechá naítaný BRT_RND_R
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
            if (tkn->type != COMMA) {
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }
        if (!(tkn->type == ID || tkn->type == UNDERSCORE)) {
            logErrSyntax(tkn, "parameter name or underscore");
            return SYN_ERR;
        }
        TRY_OR_EXIT(nextToken());
        if (tkn->type != ID) {
            logErrSyntax(tkn, "parameter identifier");
            return SYN_ERR;
        }
        TRY_OR_EXIT(nextToken());
        if (tkn->type != COLON) {
            logErrSyntax(tkn, "':'");
            return SYN_ERR;
        }
        TSData_T data_el; // TODO zmenit
        TRY_OR_EXIT(parseDataType(&data_el));
        params++;
        TRY_OR_EXIT(nextToken());
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
    if (tkn->type != ID) {
        logErrSyntax(tkn, "function identifier");
        return SYN_ERR;
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) {
        logErrSyntax(tkn, "'('");
        return SYN_ERR;
    }

    TRY_OR_EXIT(parseFunctionSignature());

    TRY_OR_EXIT(nextToken());

    if (tkn->type == ARROW) {
        //<FN_RET_TYPE>   ->  "->" <TYPE>
        TSData_T data_el; // TODO zmenit
        TRY_OR_EXIT(parseDataType(&data_el));
        TRY_OR_EXIT(nextToken());
    }

    if (tkn->type == BRT_CUR_L)
    {
        // <FN_RET_TYPE>   ->  €
        TRY_OR_EXIT(nextToken());
    }
    else {
        logErrSyntax(tkn, "'{'");
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
        if (tkn->type != ID) {
            logErrSyntax(tkn, "identifier");
            return SYN_ERR;
        }
        // TODO
        break;
    case ID:
    case BRT_RND_L:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
    case NIL:
        // <COND> ->  exp
        char exp_type;
        parseExpression(&exp_type);
        break;
    default:
        logErrSyntax(tkn, "let or an expression");
        return SYN_ERR;
        break;
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_CUR_L) {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }
    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != ELSE) {
        logErrSyntax(tkn, "else");
        return SYN_ERR;
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_CUR_L) {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }
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
    if (!(tkn->type == ID || tkn->type == BRT_RND_L)) {
        logErrSyntax(tkn, "expression");
        return SYN_ERR;
    }

    char exp_type;
    parseExpression(&exp_type);
    // TODO

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_CUR_L) {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }
    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    parser_inside_loop = loop_inside_loop;
    return COMPILATION_OK;
}

/* --- FUNKCIE DEKLAROVANÉ V PARSER.H --- */

int nextToken() {
    if (tkn != NULL) destroyToken(tkn);
    tkn = getToken();
    if (tkn == NULL) return COMPILER_ERROR;
    if (tkn->type == INVALID) {
        logErrCodeAnalysis(LEX_ERR, tkn->ln, tkn->col, "invalid token");
        return LEX_ERR;
    }
    return COMPILATION_OK;
}

void saveToken() {
    storeToken(tkn);
    tkn = NULL;
}

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
        char result_type;
        token_T* first_tkn = tkn;
        tkn = NULL;
        TRY_OR_EXIT(nextToken());
        if (tkn->type == BRT_RND_L) {
            // <STAT>   ->  <CALL_FN>
            saveToken();
            tkn = first_tkn;
            TRY_OR_EXIT(parseFnCall(&result_type));
        }
        else if (tkn->type == ASSIGN) {
            // <STAT>   ->  id = <ASSIGN>
            TRY_OR_EXIT(parseAssignment(&result_type));
        }
        else {
            logErrSyntax(tkn, "'(' or '='");
            return SYN_ERR;
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
        TRY_OR_EXIT(parseTerm(&term_type)); // !!! zmeniť na parseExpression
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
        logErrSyntax(tkn, "beginning of a statement");
        return SYN_ERR;
        break;
    }

    return COMPILATION_OK;
}


/* Koniec súboru parser.c */
