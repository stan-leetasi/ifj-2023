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

bool parser_inside_fn_def = false;

/**
 * @brief Názov funkcie, ktorej definícia je práve spracovávaná
*/
static str_T fn_name;

/**
 * @brief Zoznam mien funkcií, pri ktorých treba na konci sémantickej analýzy skontrovať, či boli definované.
*/
static DLLstr_T check_def_fns;

/**
 * @brief Indikuje, či sa parser nachádza vo vnútri cykla.
*/
static bool parser_inside_loop = false;

/**
 * @brief Meno náveštia na najvrchnejší cyklus
*/
static str_T first_loop_label;

/**
 * @brief Zoznam premenných,, ktoré musia byť dekalrované pred prvým nespracovaným cyklom
*/
static DLLstr_T variables_declared_inside_loop;


/* ----------- PRIVATE FUNKCIE ----------- */

#define PERFORM_RISKY_OP(operation)         \
    do {               \
        if(!(operation)) { \
            logErrCompilerMemAlloc(); \
            return COMPILER_ERROR;\
        }\
    } while (0)

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
 * @details Očakáva, že v globálnej premennej tkn je už načítaný prvý token argumentu
 * @param par_name
 * @param term
 * @param term_type
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnArg(str_T* par_name, str_T* term, char* term_type) {
    // <PAR_IN>        ->  id : term
    // <PAR_IN>        ->  term
    StrFillWith(par_name, StrRead(&(tkn->atr)));
    StrFillWith(term, StrRead(&(tkn->atr)));
    switch (tkn->type)
    {
    case ID:
        TRY_OR_EXIT(nextToken());
        if (tkn->type == COMMA || tkn->type == BRT_RND_R) {
            // <PAR_IN> ->  term
            // term je premenná
            StrFillWith(par_name, "_");
            TSData_T* variable = SymTabLookup(&symt, StrRead(term));
            if (variable == NULL) {
                logErrSemantic(tkn, "%s was undeclared", StrRead(term));
                return SEM_ERR_UNDEF;
            }
            if (variable->type == SYM_TYPE_FUNC) {
                logErrSemantic(tkn, "%s is a function", StrRead(term));
                return SEM_ERR_RETURN;
            }
            if (!(variable->init)) {
                logErrSemantic(tkn, "%s was uninitialized", StrRead(term));
                return SEM_ERR_UNDEF;
            }
            *term_type = variable->type;
            saveToken();
        }
        else if (tkn->type == COLON) {
            // <PAR_IN>        ->  id : term
            TRY_OR_EXIT(nextToken());
            StrFillWith(term, StrRead(&(tkn->atr)));
            TRY_OR_EXIT(parseTerm(term_type));
        }
        else {
            logErrSyntax(tkn, "',' or ':'");
            return SYN_ERR;
        }
        break;

        // <PAR_IN> ->  term
    case INT_CONST:
        StrFillWith(par_name, "_");
        *term_type = SYM_TYPE_INT;
        break;
    case DOUBLE_CONST:
        StrFillWith(par_name, "_");
        *term_type = SYM_TYPE_DOUBLE;
        break;
    case STRING_CONST:
        StrFillWith(par_name, "_");
        *term_type = SYM_TYPE_STRING;
        break;
    case NIL:
        StrFillWith(par_name, "_");
        *term_type = SYM_TYPE_NIL;
        break;
    default:
        logErrSyntax(tkn, "parameter identifier or term");
        return SYN_ERR;
    }

    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie argumentov volanej funkcie
 * @details Očakáva, že v globálnej premennej tkn je už načítaný '(', zanechá načítaný ')'
 * @param defined Značí či bola funkcia definovaná
 * @param called_before Značí, či už bola daná funkcia predtým volaná
 * @param sig Signatúra funkcie, ktorá je volaná
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnCallArgs(bool defined, bool called_before, func_sig_T* sig) {
    // <PAR_LIST>      ->  <PAR_IN> <PAR_IN_NEXT>
    // <PAR_LIST>      ->  €
    // <PAR_IN_NEXT>   ->  , <PAR_IN> <PAR_IN_NEXT>
    // <PAR_IN_NEXT>   ->  €
    // <PAR_IN>        ->  id : term
    // <PAR_IN>        ->  term

    size_t loaded_args = 0;

    char arg_type;
    str_T par_name, arg, temp;
    StrInit(&par_name);
    StrInit(&arg);
    StrInit(&temp);
    DLLstr_First(&(sig->par_names));

    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_RND_R)
    {
        if (loaded_args > 0) { // ??? dat pred kontrolu poctu argumentov ???
            if (tkn->type != COMMA) {
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }
        else {
            switch (tkn->type)
            {
            case ID:
            case INT_CONST:
            case DOUBLE_CONST:
            case STRING_CONST:
            case NIL:
                break;
            default:
                logErrSyntax(tkn, "term or parameter name");
                return SYN_ERR;
                break;
            }
        }


        if (defined || called_before) { /// ??? semnaticka > syntax chybou ???
            if (strlen(StrRead(&(sig->par_types))) <= loaded_args) {
                logErrSemantic(tkn, "too many arguments in function call");
                return SEM_ERR_FUNC;
            }
        }

        TRY_OR_EXIT(parseFnArg(&par_name, &arg, &arg_type));

        if (defined || called_before) {
            if (!isCompatibleAssign(StrRead(&(sig->par_types))[loaded_args], arg_type)) {
                logErrSemantic(tkn, "different type in function call");
                return SEM_ERR_FUNC;
            }

            DLLstr_GetValue(&(sig->par_names), &temp);
            if (strcmp(StrRead(&par_name), StrRead(&temp)) != 0) {
                logErrSemantic(tkn, "different parameter name");
                return SEM_ERR_FUNC;
            }
        }
        else {
            switch (arg_type)
            {
            case SYM_TYPE_INT:
                StrAppend(&(sig->par_types), SYM_TYPE_INT_NIL);
                break;
            case SYM_TYPE_DOUBLE:
                StrAppend(&(sig->par_types), SYM_TYPE_DOUBLE_NIL);
                break;
            case SYM_TYPE_STRING:
                StrAppend(&(sig->par_types), SYM_TYPE_STRING_NIL);
                break;
            case SYM_TYPE_NIL:
                StrAppend(&(sig->par_types), SYM_TYPE_UNKNOWN);
                break;
            default:
                StrAppend(&(sig->par_types), arg_type);
                break;
            }

            if (!DLLstr_InsertLast(&(sig->par_names), StrRead(&par_name))) {
                logErrCompilerMemAlloc();
                return COMPILER_ERROR;
            }
        }

        TRY_OR_EXIT(nextToken());
        loaded_args++;
        DLLstr_Next(&(sig->par_names));
    }

    if (defined || called_before) {
        if (strlen(StrRead(&(sig->par_types))) != loaded_args) {
            logErrSemantic(tkn, "different count of arguments in function call");
            return SEM_ERR_FUNC;
        }
    }

    StrDestroy(&par_name);
    StrDestroy(&arg);
    StrDestroy(&temp);

    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie volania funkcie
 * @details Očakáva, že v globálnej premennej tkn je už načítaný identifikátor volanej funkcie
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnCall(char* result_type) {
    // <CALL_FN>       ->  id ( <PAR_LIST> )

    TSData_T* fn = SymTabLookupGlobal(&symt, StrRead(&(tkn->atr)));
    bool called_before = fn != NULL;
    if (fn == NULL)
    {
        fn = SymTabCreateElement(StrRead(&(tkn->atr)));
        if (fn == NULL) return COMPILER_ERROR;
        if (!SymTabInsertGlobal(&symt, fn)) {
            SymTabDestroyElement(fn);
            return COMPILER_ERROR;
        }
        StrFillWith(&(fn->codename), StrRead(&(tkn->atr)));
        fn->type = SYM_TYPE_FUNC;
        fn->sig = SymTabCreateFuncSig();
        if (fn->sig == NULL) {
            return COMPILER_ERROR;
        }
        fn->sig->ret_type = SYM_TYPE_UNKNOWN;
        fn->let = false;
        fn->init = false;
    }
    else {
        if (fn->init) {

        }
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) {
        logErrSyntax(tkn, "'('");
        return SYN_ERR;
    }

    TRY_OR_EXIT(parseFnCallArgs(fn->init, called_before, fn->sig));

    *result_type = fn->sig->ret_type;

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
        logErrCompilerMemAlloc();
        return COMPILER_ERROR;
    }
    variable->init = false;
    variable->let = let;
    variable->sig = NULL;
    variable->type = SYM_TYPE_UNKNOWN;

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
            // implicitne inicializované na nil v prípade dátového typu zahrňujúceho nil
            switch (variable->type)
            {
            case SYM_TYPE_INT_NIL:
            case SYM_TYPE_DOUBLE_NIL:
            case SYM_TYPE_STRING_NIL:
                variable->init = true;
                break;
            default:
                break;
            }
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
    if (!SymTabInsertLocal(&symt, variable)) return COMPILER_ERROR;

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
 * @param compare_and_update Keď true: režim porovnávania s aktuálne zaznamenanou signatúrou volania, inak vytvára novú signatúru.
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFunctionSignature(bool compare_and_update, func_sig_T* sig) {
    // <FN_SIG>        ->  <FN_PAR> <FN_PAR_NEXT>
    // <FN_SIG>        ->  €

    size_t loaded_params = 0;
    DLLstr_First(&(sig->par_names));
    DLLstr_First(&(sig->par_ids));

    str_T tmp;
    StrInit(&tmp);

    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_RND_R)
    {
        // <FN_PAR_NEXT>   ->  , <FN_PAR> <FN_PAR_NEXT>
        // <FN_PAR_NEXT>   ->  €
        // <FN_PAR>        ->  id id : <TYPE>
        // <FN_PAR>        ->  _  id : <TYPE>
        if (loaded_params > 0) {
            if (tkn->type != COMMA) {
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }

        // ??? musia byť rôzne názvy parametrov ?
        if (tkn->type == ID || tkn->type == UNDERSCORE) {
            if (compare_and_update) {
                DLLstr_GetValue(&(sig->par_names), &tmp);
                if (strcmp(StrRead(&tmp), StrRead(&(tkn->atr))) != 0) {
                    logErrSemantic(tkn, "different parameter name");
                    return SEM_ERR_FUNC;
                }                
            }
            else {
                PERFORM_RISKY_OP(DLLstr_InsertLast(&(sig->par_names), StrRead(&(tkn->atr))));
            }
        }
        else {
            logErrSyntax(tkn, "parameter name or underscore");
            return SYN_ERR;
        }

        TRY_OR_EXIT(nextToken());
        if (tkn->type == ID) {
            compare_and_update ? DLLstr_GetValue(&(sig->par_names), &tmp) : DLLstr_GetLast(&(sig->par_names), &tmp);
            if (strcmp(StrRead(&tmp), StrRead(&(tkn->atr))) == 0) {
                logErrSemantic(tkn, "parameter name and identifier must be different");
                return SEM_ERR_FUNC;
            }  
            PERFORM_RISKY_OP(DLLstr_InsertLast(&(sig->par_ids), StrRead(&(tkn->atr))));
        }
        else {
            logErrSyntax(tkn, "parameter identifier");
            return SYN_ERR;
        }

        TRY_OR_EXIT(nextToken());
        if (tkn->type != COLON) {
            logErrSyntax(tkn, "':'");
            return SYN_ERR;
        }

        char data_type;
        TRY_OR_EXIT(parseDataType(&data_type));
        if (compare_and_update) {
            // TODO kontrola 
            sig->par_types.data[loaded_params] = data_type;
        }
        else {
            StrAppend(&(sig->par_types), data_type);
        }

        loaded_params++;
        DLLstr_Next(&(sig->par_names));
        DLLstr_Next(&(sig->par_ids));
        TRY_OR_EXIT(nextToken());
    }

    // !!! TODO kontrola názvov rôznych id parametrov 
    StrDestroy(&tmp);

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

    TSData_T* fn = SymTabLookupGlobal(&symt, StrRead(&(tkn->atr)));
    bool already_called = fn != NULL;
    if (fn == NULL) {
        fn = SymTabCreateElement(StrRead(&(tkn->atr)));
        if (fn == NULL) return COMPILER_ERROR;
        if (!SymTabInsertGlobal(&symt, fn)) {
            SymTabDestroyElement(fn);
            return COMPILER_ERROR;
        }
        StrFillWith(&(fn->codename), StrRead(&(tkn->atr)));
        fn->type = SYM_TYPE_FUNC;
        fn->sig = SymTabCreateFuncSig();
        if (fn->sig == NULL) {
            return COMPILER_ERROR;
        }
        fn->sig->ret_type = SYM_TYPE_VOID;
    }
    else {
        if (fn->type != SYM_TYPE_FUNC) {
            logErrSemantic(tkn, "identifier is already used as a variable");
            return SEM_ERR_OTHER;
        }
        if (fn->init) {
            logErrSemantic(tkn, "function was already defined");
            return SEM_ERR_OTHER;
        }
    }

    StrFillWith(&fn_name, fn->id);

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) {
        logErrSyntax(tkn, "'('");
        return SYN_ERR;
    }

    TRY_OR_EXIT(parseFunctionSignature(already_called, fn->sig));

    TRY_OR_EXIT(nextToken());

    fn->sig->ret_type = SYM_TYPE_VOID;
    if (tkn->type == ARROW) {
        //<FN_RET_TYPE>   ->  "->" <TYPE>
        TRY_OR_EXIT(parseDataType(&(fn->sig->ret_type)));
        TRY_OR_EXIT(nextToken());
    }

    if (tkn->type == BRT_CUR_L) {
        // <FN_RET_TYPE>   ->  €
        TRY_OR_EXIT(nextToken());
    }
    else {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }

    // priprava parametrov do TS
    if (!SymTabAddLocalBlock(&symt)) return COMPILER_ERROR;
    DLLstr_First(&(fn->sig->par_ids));
    str_T par_id;
    StrInit(&par_id);
    for (size_t i = 0; StrRead(&(fn->sig->par_types))[i] != '\0'; i++) {
        DLLstr_GetValue(&(fn->sig->par_ids), &par_id);
        TSData_T* par = SymTabCreateElement(StrRead(&par_id));
        if (par == NULL) {
            logErrCompilerMemAlloc();
            return COMPILER_ERROR;
        }
        PERFORM_RISKY_OP(SymTabInsertLocal(&symt, par));
        par->init = true;
        par->let = true;
        par->type = StrRead(&(fn->sig->par_types))[i];
        // TODO par->codename
        DLLstr_Next(&(fn->sig->par_ids));
    }
    StrDestroy(&par_id);

    if (!SymTabAddLocalBlock(&symt)) return COMPILER_ERROR;
    while (tkn->type != BRT_CUR_R) {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }
    SymTabRemoveLocalBlock(&symt);

    SymTabRemoveLocalBlock(&symt); // blok parametrov

    fn->init = true;
    parser_inside_fn_def = code_inside_fn_def;
    return COMPILATION_OK;
}

/**
 * @brief Pravidlo pre spracovanie vrátenia návratovej hodnoty funkcie - return
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token RETURN
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseReturn() {
    TRY_OR_EXIT(nextToken());

    if (!parser_inside_fn_def) {
        logErrSemantic(tkn, "return outside function definition");
        return SEM_ERR_OTHER;
    }
    char result_type;
    TRY_OR_EXIT(parseExpression(&result_type));
    TSData_T* fn = SymTabLookupGlobal(&symt, StrRead(&fn_name));
    if (!isCompatibleAssign(fn->sig->ret_type, result_type)) {
        logErrSemanticFn(fn->id, "different return type");
        return SEM_ERR_FUNC;
    }
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
        let_variable->type = variable->type; // ??? treba test či môže obsahovať nil hodnotu if ()
        switch (variable->type)
        {
        case SYM_TYPE_INT_NIL:
            let_variable->type = SYM_TYPE_INT;
            break;
        case SYM_TYPE_DOUBLE_NIL:
            let_variable->type = SYM_TYPE_DOUBLE;
            break;
        case SYM_TYPE_STRING_NIL:
            let_variable->type = SYM_TYPE_STRING;
            break;
        default:
            // ??? treba test či môže obsahovať nil hodnotu if ()
            break;
        }
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
        // dest je dátový typ zahrňujúci nil
        return dest == src;
        break;
    }
    return false;
}

int nextToken() {
    if (tkn != NULL) destroyToken(tkn);
    tkn = getToken();
    if (tkn == NULL) return COMPILER_ERROR;
    logErrUpdateTokenInfo(tkn);
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

    StrInit(&fn_name);

    DLLstr_Init(&check_def_fns);

    StrInit(&first_loop_label);
    DLLstr_Init(&variables_declared_inside_loop);

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
            TSData_T* variable = SymTabLookup(&symt, StrRead(&(first_tkn->atr)));
            if (variable == NULL) {
                logErrSemantic(first_tkn, "%s was undeclared", StrRead(&(first_tkn->atr)));
                return SEM_ERR_REDEF;
            }
            if (variable->init && variable->let) {
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
        TRY_OR_EXIT(parseStatBlock());
        break;
    case FUNC:
        // <STAT> ->  func id ( <FN_SIG> ) <FN_RET_TYPE> { <PROG> }
        TRY_OR_EXIT(parseFunction());
        break;
    case RETURN:
        // <STAT>      ->  return <RET_VAL>
        TRY_OR_EXIT(parseReturn());
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

void destroyParser() {
    if(tkn != NULL) {
        destroyToken(tkn);
        tkn = NULL;
    }
    SymTabDestroy(&symt);

    StrDestroy(&fn_name);
    DLLstr_Dispose(&check_def_fns);

    StrDestroy(&first_loop_label);
    DLLstr_Dispose(&variables_declared_inside_loop);

    DLLstr_Dispose(&code_main);
    DLLstr_Dispose(&code_fn);
}

/* Koniec súboru parser.c */
