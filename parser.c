/** Projekt IFJ2023
 * @file parser.c
 * @brief Syntaktický a sémantický anayzátor
 * @author Michal Krulich (xkruli03)
 * @date 12.11.2023
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
 * @brief Prekonvertuje dátový typ zahrňujúci nil na dátový typ nezahrňujúci nil.
*/
char convertNilTypeToNonNil(char nil_type) {
    switch (nil_type)
    {
    case SYM_TYPE_INT_NIL:
        return SYM_TYPE_INT;
    case SYM_TYPE_DOUBLE_NIL:
        return SYM_TYPE_DOUBLE;
    case SYM_TYPE_STRING_NIL:
        return SYM_TYPE_STRING;
    default:
        break;
    }
    return nil_type;
}

/**
 * @brief Pravidlo pre spracovanie dátového typu, zapíše do data_type spracovaný typ
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token COLON alebo ARROW
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseDataType(char* data_type) {
    // <TYPE>  ->  {Int, Double, String} <QUESTMARK>
    TRY_OR_EXIT(nextToken());
    switch (tkn->type)
    {
    case INT_TYPE:
        *data_type = SYM_TYPE_INT;
        break;
    case DOUBLE_TYPE:
        *data_type = SYM_TYPE_DOUBLE;
        break;
    case STRING_TYPE:
        *data_type = SYM_TYPE_STRING;
        break;
    default:
        logErrSyntax(tkn, "data type");
        return SYN_ERR;
        break;
    }
    TRY_OR_EXIT(nextToken());
    if (tkn->type == QUEST_MARK) {
        switch (*data_type)
        {
        case SYM_TYPE_INT:
            *data_type = SYM_TYPE_INT_NIL;
            break;
        case SYM_TYPE_DOUBLE:
            *data_type = SYM_TYPE_DOUBLE_NIL;
            break;
        case SYM_TYPE_STRING:
            *data_type = SYM_TYPE_STRING_NIL;
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
        TSData_T* variable = SymTabLookup(&symt, StrRead(&(tkn->atr)));
        if (variable == NULL) {
            logErrSemantic(tkn, "%s was undeclared", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
        if (variable->type == SYM_TYPE_FUNC) {
            logErrSemantic(tkn, "%s is a function", StrRead(&(tkn->atr)));
            return SEM_ERR_RETURN;
        }
        if (!(variable->init)) {
            logErrSemantic(tkn, "%s was uninitialized", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
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
        if (args > 0) {
            if (tkn->type != COMMA) {
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }
        switch (tkn->type)
        {
        case ID:
            TRY_OR_EXIT(nextToken());
            if (tkn->type == COMMA) {
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
int parseFnCall(char* result_type) {
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
        TRY_OR_EXIT(parseExpression(result_type));
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
            TRY_OR_EXIT(parseExpression(result_type));
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
    if (SymTabLookupLocal(&symt, StrRead(&(tkn->atr))) != NULL) {
        logErrSemantic(tkn, "%s is already declared in this block", StrRead(&(tkn->atr)));
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

    if (!SymTabInsertLocal(&symt, variable)) return COMPILER_ERROR;

    TRY_OR_EXIT(nextToken());
    switch (tkn->type)
    {
    case COLON:
        // <DEF_VAR>  ->  : <TYPE> <INIT_VAL>
        TRY_OR_EXIT(parseDataType(&(variable->type)));
        TRY_OR_EXIT(nextToken());
        if (tkn->type == ASSIGN) {
            // <INIT_VAL>  ->  = <ASSIGN>
            char assign_type = SYM_TYPE_UNKNOWN;
            TRY_OR_EXIT(parseAssignment(&assign_type));
            variable->init = true;
            if (!isCompatibleAssign(variable->type, assign_type)) {
                logErrSemantic(tkn, "incompatible data types");
                return SEM_ERR_TYPE;
            }
        }
        else {
            // <INIT_VAL>  ->  €
            saveToken();
        }
        break;
    case ASSIGN:
        // <DEF_VAR>   ->  = <ASSIGN>
        TRY_OR_EXIT(parseAssignment(&(variable->type)));
        variable->init = true;
        // unknown TODO
        break;
    default:
        logErrSyntax(tkn, "':' or '='");
        return SYN_ERR;
        break;
    }

    //TRY_OR_EXIT(genUniqVar("GF", variable->id, &(variable->codename)));
    // TRY_OR_EXIT(genCode("DEFVAR", StrRead(&(variable->codename)), NULL, NULL));
    // pops TRY_OR_EXIT()

    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie bloku kódu
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token BRT_CUR_L, zanechá načítaný BRT_CUR_R
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseStatBlock() {
    // <STAT>      ->  { <PROG> }
    if (tkn->type != BRT_CUR_L) {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }
    if (!SymTabAddLocalBlock(&symt)) return COMPILER_ERROR;
    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }
    SymTabRemoveLocalBlock(&symt);
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie definície parametrov funkcie, končí načítaním pravej zátvorky
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token BRT_RND_L, zanechá načítaný BRT_RND_R
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
        TRY_OR_EXIT(parseDataType(&(data_el.type)));
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
        TRY_OR_EXIT(parseDataType(&(data_el.type)));
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

    TSData_T* let_variable = NULL;
    switch (tkn->type)
    {
    case LET:
        //  <COND> ->  let id
        TRY_OR_EXIT(nextToken());
        if (tkn->type != ID) {
            logErrSyntax(tkn, "identifier");
            return SYN_ERR;
        }
        TSData_T* variable = SymTabLookup(&symt, StrRead(&(tkn->atr)));
        if (variable == NULL) {
            logErrSemantic(tkn, "%s was undeclared", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
        if (variable->type == SYM_TYPE_FUNC) {
            logErrSemantic(tkn, "%s is a function", StrRead(&(tkn->atr)));
            return SEM_ERR_RETURN;
        }
        if (!(variable->init)) {
            logErrSemantic(tkn, "%s was uninitialized", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }

        // ??? treba test či môže obsahovať nil hodnotu if ()

        if (!SymTabAddLocalBlock(&symt)) {
            return COMPILER_ERROR;
        }
        let_variable = SymTabCreateElement(StrRead(&(tkn->atr)));
        if (let_variable == NULL)
        {
            return COMPILER_ERROR;
        }
        let_variable->init = variable->init;
        let_variable->let = variable->let;
        let_variable->sig = NULL;
        let_variable->type = variable->type;
        StrFillWith(&(let_variable->codename), StrRead(&(variable->codename)));

        if (!SymTabInsertLocal(&symt, let_variable)) return COMPILER_ERROR;
        // TODO
        break;
    case ID:
    case BRT_RND_L:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
    case NIL:
        // <COND> ->  exp
        char exp_type = SYM_TYPE_UNKNOWN; // ???
        TRY_OR_EXIT(parseExpression(&exp_type));
        if (exp_type != SYM_TYPE_BOOL && exp_type != SYM_TYPE_UNKNOWN) {
            logErrSemantic(tkn, "condition must return a bool");
            return SEM_ERR_TYPE;
        }
        break;
    default:
        logErrSyntax(tkn, "let or an expression");
        return SYN_ERR;
        break;
    }

    TRY_OR_EXIT(nextToken());
    TRY_OR_EXIT(parseStatBlock());
    if (let_variable != NULL) SymTabRemoveLocalBlock(&symt);

    TRY_OR_EXIT(nextToken());
    if (tkn->type != ELSE) {
        logErrSyntax(tkn, "else");
        return SYN_ERR;
    }

    TRY_OR_EXIT(nextToken());
    TRY_OR_EXIT(parseStatBlock());

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

    switch (tkn->type)
    {
    case ID:
    case BRT_RND_L:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
    case NIL:
        break;
    default:
        logErrSyntax(tkn, "expression");
        return SYN_ERR;
    }

    char exp_type = SYM_TYPE_UNKNOWN; // ???
    TRY_OR_EXIT(parseExpression(&exp_type));
    if (exp_type != SYM_TYPE_BOOL && exp_type != SYM_TYPE_UNKNOWN) {
        logErrSemantic(tkn, "condition must return a bool");
        return SEM_ERR_TYPE;
    }

    TRY_OR_EXIT(nextToken());
    TRY_OR_EXIT(parseStatBlock());

    parser_inside_loop = loop_inside_loop;
    return COMPILATION_OK;
}

/* --- FUNKCIE DEKLAROVANÉ V PARSER.H --- */

bool isCompatibleAssign(char dest, char src) {
    if (dest == SYM_TYPE_UNKNOWN) return true;
    if (src == SYM_TYPE_VOID) return false;
    if (src == SYM_TYPE_UNKNOWN) return true;
    if (src == SYM_TYPE_NIL) {
        switch (dest)
        {
        case SYM_TYPE_INT_NIL:
        case SYM_TYPE_DOUBLE_NIL:
        case SYM_TYPE_STRING_NIL:
            return true;
        default:
            return false;
        }
    }
    switch (src)
    {
    case SYM_TYPE_INT:
        return dest == SYM_TYPE_INT || dest == SYM_TYPE_INT_NIL;
    case SYM_TYPE_DOUBLE:
        return dest == SYM_TYPE_DOUBLE || dest == SYM_TYPE_DOUBLE_NIL;
    case SYM_TYPE_STRING:
        return dest == SYM_TYPE_STRING || dest == SYM_TYPE_STRING_NIL;
    case SYM_TYPE_BOOL:
        return dest == SYM_TYPE_BOOL;
    default:
        break;
    }
    return false;
}

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
            TSData_T *variable = SymTabLookup(&symt, StrRead(&(first_tkn->atr)));
            if (variable == NULL) {
                logErrSemantic(first_tkn, "%s was undeclared", StrRead(&(first_tkn->atr)));
                return SEM_ERR_REDEF;
            }
            if(variable->init && variable->let) {
                logErrSemantic(first_tkn, "%s is unmodifiable and was already initialised", StrRead(&(first_tkn->atr)));
                return SEM_ERR_OTHER;
            }
            TRY_OR_EXIT(parseAssignment(&result_type));
            if (!isCompatibleAssign(variable->type, result_type)) {
                logErrSemantic(first_tkn, "incompatible data types");
                return SEM_ERR_TYPE;
            }
            destroyToken(first_tkn);
        }
        else {
            logErrSyntax(tkn, "'(' or '='");
            return SYN_ERR;
        }
        break;
    case BRT_CUR_L:
        // <STAT>  ->  { <PROG> }
        TRY_OR_EXIT(nextToken());
        TRY_OR_EXIT(parseStatBlock());
        break;
    case FUNC:
        // <STAT> ->  func id ( <FN_SIG> ) <FN_RET_TYPE> { <PROG> }
        TRY_OR_EXIT(parseFunction());
        break;
    case RETURN:
        // <STAT>      ->  return <RET_VAL>
        TRY_OR_EXIT(nextToken());
        char term_type;
        TRY_OR_EXIT(parseExpression(&term_type));
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
