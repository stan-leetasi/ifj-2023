/** Projekt IFJ2023
 * @file parser.c
 * @brief Syntaktický a sémantický anayzátor
 * @author Michal Krulich (xkruli03)
 * @date 18.11.2023
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
 * @brief Zistí, či sa v zozname nachádzajú medzi sebou rôzne reťazce (pozor! reťazce "_" sú ignorované), resp. nie je možné nájsť dva totožné.
 * @param list zoznam
 * @param is_unique Ukazateľ na bool, kde sa zapíše false ak existujú dve položky zoznamu s rovnakým reťazcom, inak true
 * @warning Mení aktivitu zoznamu.
 * @return true v prípade úspechu, false v prípade chyby programu
*/

bool listHasUniqueValues(DLLstr_T* list, bool* is_unique) {
    str_T a, b; // reťazce položiek A a B
    StrInit(&a);
    StrInit(&b);

    DLLstr_First(list);
    for (size_t i = 0; DLLstr_IsActive(list); i++) {
        for (size_t j = 0; j < i; j++) {
            DLLstr_Next(list);
        }
        if (!DLLstr_IsActive(list)) break;
        if (!DLLstr_GetValue(list, &a)) return false; // načítanie položky A
        if (strcmp(StrRead(&a), "_") != 0) { // reťazce "_" sú ignorované
            DLLstr_Next(list);
            while (DLLstr_IsActive(list)) { // porovnávanie s položkami napravo od položky A
                if (!DLLstr_GetValue(list, &b)) return false; // načítanie položky B
                if (strcmp(StrRead(&a), StrRead(&b)) == 0) {
                    // totožné reťazce
                    StrDestroy(&a);
                    StrDestroy(&b);
                    *is_unique = false;
                    return true;
                }
                DLLstr_Next(list);
            }
        }
        DLLstr_First(list);
    }

    StrDestroy(&a);
    StrDestroy(&b);
    *is_unique = true;
    return true;
}

/**
 * @brief Zistí či funkcia je vstavaná.
*/
bool isBuiltInFunction(char * function_name) {
    static size_t total_num_of_bif = 10;
    static char *built_in_functions[] = {
        "readString", "readInt", "readDouble",
        "write", "Int2Double", "Double2Int",
        "length", "substring", "ord", "chr"
    };
    for (size_t i = 0; i < total_num_of_bif; i++) {
        if(strcmp(function_name, built_in_functions[i]) == 0) return true;
    }
    return false;
}

/**
 * @brief Vloží danú signatúru funkcie do TS
*/
void loadSingleBIFnSig(char* name, size_t count_par, char ret_type,
    char* par_names[], char* par_types) {
    
    TSData_T* fn = SymTabCreateElement(name);
    fn->type = SYM_TYPE_FUNC;
    StrFillWith(&(fn->codename), name);
    fn->init = true;
    fn->sig = SymTabCreateFuncSig();
    fn->sig->ret_type = ret_type;
    for(size_t i=0; i<count_par; i++) {
        DLLstr_InsertLast(&(fn->sig->par_names), par_names[i]);
    }
    StrFillWith(&(fn->sig->par_types), par_types);

    SymTabInsertGlobal(&symt, fn);
}


/**
 * @brief Načíta signatúry vstavaných funkcií do TS
*/
void loadBuiltInFunctionSignatures() {
    loadSingleBIFnSig("readString", 0, SYM_TYPE_STRING_NIL, NULL, "");
    loadSingleBIFnSig("readInt", 0, SYM_TYPE_INT_NIL, NULL, "");
    loadSingleBIFnSig("readDouble", 0, SYM_TYPE_DOUBLE_NIL, NULL, "");

    loadSingleBIFnSig("write", 0, SYM_TYPE_VOID, NULL, "");

    char *empty_param[] = { "_" };
    loadSingleBIFnSig("Int2Double", 1, SYM_TYPE_DOUBLE, empty_param, "i");
    loadSingleBIFnSig("Double2Int", 1, SYM_TYPE_INT, empty_param, "d");
    loadSingleBIFnSig("length", 1, SYM_TYPE_INT, empty_param, "s");
    loadSingleBIFnSig("ord", 1, SYM_TYPE_INT, empty_param, "s");
    loadSingleBIFnSig("chr", 1, SYM_TYPE_STRING, empty_param, "i");

    char *substring_par_names[] = {"of", "startingAt", "endingBefore"};
    loadSingleBIFnSig("substring", 3, SYM_TYPE_STRING_NIL, substring_par_names, "sii");
}

/**
 * Stav tkn:
 *  - pred volaním: COLON alebo ARROW
 *  - po volaní:    NULL
 *
 * @brief Pravidlo pre spracovanie dátového typu, zapíše do data_type spracovaný typ
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
    if (tkn->type == QUEST_MARK) { // dátový typ zahrnǔjúci nil: <TYP>?
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
    else { // za dátovým typom nebol token '?', token je vrátený naspäť
        saveToken();
    }
    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: token termu
 *  - po volaní:    token termu
 *
 * @brief Pravidlo pre spracovanie termu
 * @param term_type Dátový typ termu
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseTerm(char* term_type) {
    switch (tkn->type)
    {
    case ID: // premenná
        TSData_T* variable = SymTabLookup(&symt, StrRead(&(tkn->atr)));
        if (variable == NULL) {
            // v TS nie je záznam s daným identifikátorom => nedeklarovaná premenná
            logErrSemantic(tkn, "%s was undeclared", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
        if (variable->type == SYM_TYPE_FUNC) {
            // identifikátor označuje funkciu
            logErrSemantic(tkn, "%s is a function", StrRead(&(tkn->atr)));
            return SEM_ERR_RETURN;
        }
        if (!(variable->init)) {
            // premenná nebola inicializovaná
            logErrSemantic(tkn, "%s was uninitialized", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
        *term_type = variable->type;
        break;
    case INT_CONST: // konštanty
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
        // nie je to term
        logErrSyntax(tkn, "term");
        return SYN_ERR;
        break;
    }
    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: prvý token argumentu
 *  - po volaní:    NULL alebo term
 *
 * @brief Pravidlo pre spracovanie argumentu volanej funkcie, pričom cez svoje parametre vráti informácie o načítanom argumente.
 * @param par_name  načítaný názov parametru
 * @param term_type dátový typ termu
 * @param bif_name Ak volaná funkcia nie je vstavaná, potom NULL, inak názov vstavanej funkcie.
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnArg(str_T* par_name, char* term_type, char *bif_name) {
    // <PAR_IN>        ->  id : term
    // <PAR_IN>        ->  term
    StrFillWith(par_name, StrRead(&(tkn->atr)));
    switch (tkn->type)
    {
    case ID:
        TRY_OR_EXIT(nextToken());
        if (tkn->type == COMMA || tkn->type == BRT_RND_R) {
            // <PAR_IN> ->  term
            // term je premenná a funkcia nemá názov pre parameter
            TSData_T* variable = SymTabLookup(&symt, StrRead(par_name));
            if (variable == NULL) {
                logErrSemantic(tkn, "%s was undeclared", StrRead(par_name));
                return SEM_ERR_UNDEF;
            }
            if (variable->type == SYM_TYPE_FUNC) {
                logErrSemantic(tkn, "%s is a function", StrRead(par_name));
                return SEM_ERR_RETURN;
            }
            if (!(variable->init)) {
                logErrSemantic(tkn, "%s was uninitialized", StrRead(par_name));
                return SEM_ERR_UNDEF;
            }
            *term_type = variable->type;
            StrFillWith(par_name, "_"); // funkcia má vynechaný názvo pre parameter
            saveToken();
        }
        // inak prvý token musí byť názov parametra
        else if (tkn->type == COLON) {
            // <PAR_IN>        ->  id : term
            TRY_OR_EXIT(nextToken());
            TRY_OR_EXIT(parseTerm(term_type));
        }
        else {
            logErrSyntax(tkn, "',' or ':'");
            return SYN_ERR;
        }
        break;
    case INT_CONST: // <PAR_IN> ->  term , kde term je konštanta
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
 * Stav tkn:
 *  - pred volaním: BRT_RND_L '('
 *  - po volaní:    BRT_RND_R ')'
 *
 * @brief Pravidlo pre spracovanie argumentov volanej funkcie
 * @param defined Značí či bola funkcia definovaná
 * @param called_before Značí, či už bola daná funkcia predtým volaná
 * @param sig Signatúra funkcie, ktorá je volaná
 * @param bif_name Ak volaná funkcia nie je vstavaná, potom NULL, inak názov vstavanej funkcie.
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnCallArgs(bool defined, bool called_before, func_sig_T* sig,
        char *bif_name) {
    // <PAR_LIST>      ->  <PAR_IN> <PAR_IN_NEXT>
    // <PAR_LIST>      ->  €
    // <PAR_IN_NEXT>   ->  , <PAR_IN> <PAR_IN_NEXT>
    // <PAR_IN_NEXT>   ->  €

    size_t loaded_args = 0; // počet načítaných argumentov

    char arg_type;  // typ aktuálne načítaného argumentu
    str_T par_name, temp; // názov parametru, pomocné reťazcové úložisko
    StrInit(&par_name);
    StrInit(&temp);
    DLLstr_First(&(sig->par_names));

    bool write_function = false;
    if (bif_name != NULL) {
        if (strcmp(bif_name, "write") == 0) write_function = true;
    }

    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_RND_R)
        // ( argument1, argument2, ..., argumentN )
    {
        // pred každým argumentom, okrem prvého, musí nasledovať čiarka
        if (loaded_args > 0) { // ??? dat pred kontrolu poctu argumentov ???
            if (tkn->type != COMMA) {
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }
        else {
            /*  Táto extra syntaktická kokontrola zaručuje vyššiu prioritu
                syntaktickej chyby pred sémantickou. */
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

        // kontrola, či nie je funkcia volaná s viacerými argumentami
        if ((defined || called_before) && !write_function) { /// ??? semnaticka > syntax chybou ???
            if (strlen(StrRead(&(sig->par_types))) <= loaded_args) {
                logErrSemantic(tkn, "too many arguments in function call");
                return SEM_ERR_FUNC;
            }
        }

        TRY_OR_EXIT(parseFnArg(&par_name, &arg_type, bif_name)); // príkaz na spracovanie jedného argumentu

        // sémantická kontrola argumentu
        if (write_function) {
            if (strcmp(StrRead(&par_name), "_") != 0) {
                logErrSemantic(tkn, "function write does not use parameter names");
                return SEM_ERR_FUNC;
            }
        }
        else if (defined || called_before) {
            // kontrola dátového typu argumentu s predpisom funkcie
            if (!isCompatibleAssign(StrRead(&(sig->par_types))[loaded_args], arg_type)) {
                logErrSemantic(tkn, "different type in function call");
                return SEM_ERR_FUNC;
            }

            // kontrola názvu parametra s predpisom
            DLLstr_GetValue(&(sig->par_names), &temp);
            if (strcmp(StrRead(&par_name), StrRead(&temp)) != 0) {
                logErrSemantic(tkn, "different parameter name");
                return SEM_ERR_FUNC;
            }
        }
        else {  // funkcia ešte nebola volaná alebo definovaná, preto sa zapíšu informácie z jej prvého volania do predpisu
            switch (arg_type)
            {
                /*  Aj keď je predávaný argument typu nezahrňujúci nil, môže
                    funkcia mať v neskošej definícii v predpise typ zahrňujúci nil.
                */
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

            // zápis názvu parametru
            if (!DLLstr_InsertLast(&(sig->par_names), StrRead(&par_name))) {
                logErrCompilerMemAlloc();
                return COMPILER_ERROR;
            }
        }

        TRY_OR_EXIT(nextToken());
        loaded_args++;
        DLLstr_Next(&(sig->par_names));
    }

    if ((defined || called_before) && !write_function) { // kontrola, či bola funkcia zavolaná so správnym počtom argumentov
        if (strlen(StrRead(&(sig->par_types))) != loaded_args) {
            logErrSemantic(tkn, "different count of arguments in function call");
            return SEM_ERR_FUNC;
        }
    }

    // dealokácia pomocných reťazcov
    StrDestroy(&par_name);
    StrDestroy(&temp);

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: identifikátor volanej funkcie
 *  - po volaní:    BRT_RND_R
 * 
 * @brief Pravidlo pre spracovanie volania funkcie
 * @param result_type návratový typ funkcie
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFnCall(char* result_type) {
    // <CALL_FN>       ->  id ( <PAR_LIST> )

    // získanie informácii o funkcii z TS
    TSData_T* fn = SymTabLookupGlobal(&symt, StrRead(&(tkn->atr)));
    bool called_before = fn != NULL;
    bool built_in_fn = false;
    if (fn == NULL) // funkcia nebola definovaná a ani volaná
    {
        // vytvorí sa o nej záznam do TS
        fn = SymTabCreateElement(StrRead(&(tkn->atr)));
        if (fn == NULL) return COMPILER_ERROR;
        StrFillWith(&(fn->codename), StrRead(&(tkn->atr)));
        fn->type = SYM_TYPE_FUNC;
        fn->sig = SymTabCreateFuncSig();
        if (fn->sig == NULL) {
            return COMPILER_ERROR;
        }
        fn->sig->ret_type = SYM_TYPE_UNKNOWN;
        fn->let = false;
        fn->init = false;
        if (!SymTabInsertGlobal(&symt, fn)) {
            SymTabDestroyElement(fn);
            return COMPILER_ERROR;
        }

        // poznačiť názov funkcie do zoznamu nedefinovaných funkcií, pre kontrolu na koniec
        if (!DLLstr_InsertLast(&check_def_fns, fn->id))
        {
            logErrCompilerMemAlloc();
            return COMPILER_ERROR;
        }
    }
    else {
        if (fn->init) {
            built_in_fn = isBuiltInFunction(fn->id);
        }
    }

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) {
        logErrSyntax(tkn, "'('");
        return SYN_ERR;
    }

    // spracovanie argumentov funkcie
    TRY_OR_EXIT(parseFnCallArgs(fn->init, called_before, fn->sig, built_in_fn ? fn->id : NULL));

    *result_type = fn->sig->ret_type;

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: ´=´
 *  - po volaní:    NULL
 *
 * @brief Pravidlo pre spracovanie priradenia
 * @param result_type Dátový typ výsledku
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseAssignment(char* result_type) {
    TRY_OR_EXIT(nextToken());
    token_T* first_tkn;
    switch (tkn->type) // treba rozlíšiť volanie funkcie a výraz
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
        /*  Identifikátor môže byť názov premennej vo výraze alebo názov funkcie.
            Preto sa treba pozrieť na ďalší token. Ak ďalší token je ľavá zátvorka,
            potom je to volanie funkcie.
        */
        first_tkn = tkn;
        tkn = NULL; // ! Bez tohoto by bol first_tkn uvoľnený v nextToken().
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
    }

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: LET alebo VAR
 *  - po volaní:    NULL
 *
 * @brief Pravidlo pre spracovanie deklarácie/definície premennej
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseVariableDecl() {
    // <STAT>  ->  {let,var} id <DEF_VAR>
    bool let = tkn->type == LET ? true : false; // (ne)modifikovateľná premenná

    TRY_OR_EXIT(nextToken());
    if (tkn->type != ID) { // musí nasledovať názov premennej
        logErrSyntax(tkn, "identifier");
        return SYN_ERR;
    }

    // kontrola, či premenná s daným identifikátorom už nebola deklarovaná v tomto bloku
    if (SymTabLookupLocal(&symt, StrRead(&(tkn->atr))) != NULL) {
        logErrSemantic(tkn, "%s is already declared in this block", StrRead(&(tkn->atr)));
        return SEM_ERR_REDEF;
    }
    // zápis novej premennej do TS
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

    // ďalej musí nasledovať dátový typ alebo priradenie
    TRY_OR_EXIT(nextToken());
    switch (tkn->type)
    {
    case COLON: // <DEF_VAR>  ->  : <TYPE> <INIT_VAL>
        // zistenie dátového typu
        TRY_OR_EXIT(parseDataType(&(variable->type)));

        // treba zistiť, či za deklaráciou dátového typu sa ešte nenachádza priradenie/inicializácia
        TRY_OR_EXIT(nextToken());
        if (tkn->type == ASSIGN) {
            // <INIT_VAL>  ->  = <ASSIGN>
            char assign_type = SYM_TYPE_UNKNOWN;
            TRY_OR_EXIT(parseAssignment(&assign_type));
            variable->init = true;

            // kontrola výsledného typu výrazu s deklarovaným dátovým typom
            if (!isCompatibleAssign(variable->type, assign_type)) {
                logErrSemantic(tkn, "incompatible data types");
                return SEM_ERR_TYPE;
            }
        }
        else { // bez počiatočnej inicializácie
            // <INIT_VAL>  ->  €
            switch (variable->type)
            {
            case SYM_TYPE_INT_NIL:
            case SYM_TYPE_DOUBLE_NIL:
            case SYM_TYPE_STRING_NIL:
                // implicitne inicializované na nil v prípade dátového typu zahrňujúceho nil
                variable->init = true;
                break;
            default:
                break;
            }
            saveToken();
        }
        break;
    case ASSIGN: // <DEF_VAR> ->  = <ASSIGN>
        TRY_OR_EXIT(parseAssignment(&(variable->type)));
        variable->init = true;

        if (variable->type == SYM_TYPE_VOID) { // priradenie hodnoty z void funkcie
            logErrSemantic(tkn, "void function does not return a value");
            return SEM_ERR_TYPE;
        }
        else if (variable->type == SYM_TYPE_NIL) {  // len z nil nie je možné odvodiť typ
            logErrSemantic(tkn, "could not deduce the data type");
            return SEM_ERR_UKN_T;
        }
        break;
    default:
        logErrSyntax(tkn, "':' or '='");
        return SYN_ERR;
        break;
    }

    //TRY_OR_EXIT(genUniqVar("GF", variable->id, &(variable->codename)));
    if (parser_inside_loop) {
        // DLLstr_InsertLast(&variables_declared_inside_loop, variable->codename);
    }
    // TRY_OR_EXIT(genCode("DEFVAR", StrRead(&(variable->codename)), NULL, NULL));
    // pops TRY_OR_EXIT()

    // vloženie záznamu o premennej do TS
    if (!SymTabInsertLocal(&symt, variable)) {
        SymTabDestroyElement(variable);
        logErrCompilerMemAlloc();
        return COMPILER_ERROR;
    }

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: BRT_CUR_L '{'
 *  - po volaní:    BRT_CUR_R '}'
 *
 * @brief Pravidlo pre spracovanie bloku kódu
 * @param had_return Ukazateľ na bool, ktorý bude značiť, či sa v bloku nachádzal príkaz return. Ak NULL, nič sa nezapisuje.
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseStatBlock(bool* had_return) {
    // <STAT>      ->  { <PROG> }

    // počiatočná ľavá zátvorka
    if (tkn->type != BRT_CUR_L) {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }

    // vytvorenie nového lokálneho bloku v TS
    if (!SymTabAddLocalBlock(&symt)) return COMPILER_ERROR;

    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_CUR_R)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }

    // vrátiť prítomnosť príkazu return a vymazať lokálny blok z TS
    if (had_return != NULL) *had_return = SymTabCheckLocalReturn(&symt);
    SymTabRemoveLocalBlock(&symt);

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: BRT_RND_L
 *  - po volaní:    BRT_RND_R
 *
 * @brief Pravidlo pre spracovanie definície parametrov funkcie, končí načítaním pravej zátvorky
 * @param compare_and_update Keď true: režim porovnávania s aktuálne zaznamenanou signatúrou volania, inak vytvára novú signatúru.
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFunctionSignature(bool compare_and_update, func_sig_T* sig) {
    // <FN_SIG>        ->  <FN_PAR> <FN_PAR_NEXT>
    // <FN_SIG>        ->  €

    size_t loaded_params = 0; // počet načítaných parametrov
    DLLstr_First(&(sig->par_names));
    DLLstr_First(&(sig->par_ids));

    str_T tmp; // pomocný reťazec
    StrInit(&tmp);

    TRY_OR_EXIT(nextToken());
    while (tkn->type != BRT_RND_R)
    {
        // <FN_PAR_NEXT>   ->  , <FN_PAR> <FN_PAR_NEXT>
        // <FN_PAR_NEXT>   ->  €
        // <FN_PAR>        ->  id id : <TYPE>
        // <FN_PAR>        ->  _  id : <TYPE>

        if (loaded_params > 0) { // pred každým parametrom, okrem prvého, musí nasledovať čiarka
            if (tkn->type != COMMA) {
                logErrSyntax(tkn, "comma");
                return SYN_ERR;
            }
            TRY_OR_EXIT(nextToken());
        }

        if (tkn->type == ID || tkn->type == UNDERSCORE) { // názov parametra musí byť identifikátor alebo '_'

            if (compare_and_update) { // funkcia bola volaná pred jej definíciou
                /* Kontrola počtu parametrov s počtom argumentov v prvom volaní. */
                if (strlen(StrRead(&(sig->par_types))) <= loaded_params) { // funkcia bola volaná s menším počtom argumentov
                    logErrSemantic(tkn, "different number of parameters in function definition and first call");
                    return SEM_ERR_FUNC;
                }

                /*  Treba skontrolovať názov parametra s prvým volaním. */
                DLLstr_GetValue(&(sig->par_names), &tmp);
                if (strcmp(StrRead(&tmp), StrRead(&(tkn->atr))) != 0) {
                    logErrSemantic(tkn, "different parameter name in definition and first call");
                    return SEM_ERR_FUNC;
                }
            }
            else { // zápis názvu parametra do predpisu funkcie
                PERFORM_RISKY_OP(DLLstr_InsertLast(&(sig->par_names), StrRead(&(tkn->atr))));
            }
        }
        else {
            logErrSyntax(tkn, "parameter name or underscore");
            return SYN_ERR;
        }

        // nasleduje identifikátor parametra vo vnútri funkcie
        TRY_OR_EXIT(nextToken());
        if (tkn->type == ID) {
            compare_and_update ? DLLstr_GetValue(&(sig->par_names), &tmp) : DLLstr_GetLast(&(sig->par_names), &tmp);

            // názov parametra a identifikátor parametra sa musia líšiť
            if (strcmp(StrRead(&tmp), StrRead(&(tkn->atr))) == 0) {
                logErrSemantic(tkn, "parameter name and identifier must be different");
                return SEM_ERR_OTHER;
            }
            PERFORM_RISKY_OP(DLLstr_InsertLast(&(sig->par_ids), StrRead(&(tkn->atr))));
        }
        else {
            logErrSyntax(tkn, "parameter identifier");
            return SYN_ERR;
        }

        // nasleduje dátový typ parametra
        TRY_OR_EXIT(nextToken()); // dvojbodka
        if (tkn->type != COLON) {
            logErrSyntax(tkn, "':'");
            return SYN_ERR;
        }
        char data_type;
        TRY_OR_EXIT(parseDataType(&data_type));
        if (compare_and_update) { // funkcia bola volaná pred jej definíciou
            // kontrola typu v definícii s typom argumentu v prvom volaní
            bool same_type = true;
            char type_before = StrRead(&(sig->par_types))[loaded_params];
            switch (data_type)
            {
            case SYM_TYPE_INT:
            case SYM_TYPE_INT_NIL:
                same_type = type_before == SYM_TYPE_INT || type_before == SYM_TYPE_INT_NIL;
                break;
            case SYM_TYPE_DOUBLE:
            case SYM_TYPE_DOUBLE_NIL:
                same_type = type_before == SYM_TYPE_DOUBLE || type_before == SYM_TYPE_DOUBLE_NIL;
                break;
            case SYM_TYPE_STRING:
            case SYM_TYPE_STRING_NIL:
                same_type = type_before == SYM_TYPE_STRING || type_before == SYM_TYPE_STRING_NIL;
                break;
            default:
                break;
            }
            if (type_before == SYM_TYPE_UNKNOWN) same_type = true;
            if (!same_type) {
                logErrSemanticFn(StrRead(&fn_name), "parameter types does not correspond to previous call");
                return SEM_ERR_FUNC;
            }
            sig->par_types.data[loaded_params] = data_type; // zapíše sa dátový typ zistení z definície
        }
        else {
            StrAppend(&(sig->par_types), data_type);
        }

        loaded_params++;
        DLLstr_Next(&(sig->par_names));
        DLLstr_Next(&(sig->par_ids));
        TRY_OR_EXIT(nextToken());
    }

    if (compare_and_update) { // kontrola počtu parametrov v definícii s počtom argumentov v prvom volaní
        if (strlen(StrRead(&(sig->par_types))) != loaded_params) {
            logErrSemantic(tkn, "different count of parameters in function definition and first call");
            return SEM_ERR_FUNC;
        }
    }

    // kontrola názvov rôznych názvov a identifikátorov parametrov 
    bool unique_names;
    if (!listHasUniqueValues(&(sig->par_names), &unique_names)) return COMPILER_ERROR;
    if (!unique_names) {
        logErrSemanticFn(StrRead(&fn_name), "parameter names don't have different names");
        return SEM_ERR_OTHER;
    }
    if (!listHasUniqueValues(&(sig->par_ids), &unique_names)) return COMPILER_ERROR;
    if (!unique_names) {
        logErrSemanticFn(StrRead(&fn_name), "parameter identifiers don't have different names");
        return SEM_ERR_OTHER;
    }

    // dealokácia pomocných štruktúr
    StrDestroy(&tmp);

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: FUNC
 *  - po volaní:    BRT_CUR_R
 *
 * @brief Pravidlo pre spracovanie definície funkcie
 * @details Očakáva, že v globálnej premennej tkn je už načítaný token FUNC
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseFunction() {
    // <STAT> ->  func id ( <FN_SIG> ) <FN_RET_TYPE> { <PROG> }
    bool code_inside_fn_def = parser_inside_fn_def;
    parser_inside_fn_def = true; // parser sa nachádza v definícií funkcie

    TRY_OR_EXIT(nextToken()); // názov funkcie
    if (tkn->type != ID) {
        logErrSyntax(tkn, "function identifier");
        return SYN_ERR;
    }

    TSData_T* fn = SymTabLookupGlobal(&symt, StrRead(&(tkn->atr)));
    bool already_called = fn != NULL; // funkcia bola volaná pred jej definíciou, pretože existuje záznam v TS
    if (fn == NULL) {
        // vytvorenie záznamu o funkcii do TS
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
        if (fn->type != SYM_TYPE_FUNC) { // existuje globálna premenná s rovnakým názvom
            logErrSemantic(tkn, "identifier is already used as a variable");
            return SEM_ERR_OTHER;
        }
        if (fn->init) { // funkcia už bola definovaná
            logErrSemantic(tkn, "function was already defined");
            return SEM_ERR_OTHER;
        }
        // v ostatných prípadoch záznam o funkcii existuje preto, lebo bola už volaná 
    }

    StrFillWith(&fn_name, fn->id); // zápis názvu aktuálne definovanej funkcie do globálnej premennej

    TRY_OR_EXIT(nextToken());
    if (tkn->type != BRT_RND_L) {
        logErrSyntax(tkn, "'('");
        return SYN_ERR;
    }

    // spracovanie predpisu, resp. definície parametrov
    TRY_OR_EXIT(parseFunctionSignature(already_called, fn->sig));

    // zistenie návratového typu funkcie
    TRY_OR_EXIT(nextToken());
    fn->sig->ret_type = SYM_TYPE_VOID;
    if (tkn->type == ARROW) {
        //<FN_RET_TYPE>   ->  "->" <TYPE>
        TRY_OR_EXIT(parseDataType(&(fn->sig->ret_type)));
        TRY_OR_EXIT(nextToken());
    } // ak nenasleduje šípka '->', potom je to void funkcia

    if (tkn->type == BRT_CUR_L) {
        // <FN_RET_TYPE>   ->  €
        TRY_OR_EXIT(nextToken());
    }
    else {
        logErrSyntax(tkn, "'{'");
        return SYN_ERR;
    }

    // Príprava parametrov pre telo funkcie
    // parametre budú vo vlastnom lokálnom bloku TS
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

    // Spracovanie tela funkcie
    if (!SymTabAddLocalBlock(&symt)) return COMPILER_ERROR;
    while (tkn->type != BRT_CUR_R) {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }
    // kontrola, či funkcia s návratovou hodnotou má všade kde je to treba, príkaz return
    if (!SymTabCheckLocalReturn(&symt) && fn->sig->ret_type != SYM_TYPE_VOID) {
        logErrSemanticFn(fn->id, "it is possible to exit function without return value");
        return SEM_ERR_FUNC;
    }
    SymTabRemoveLocalBlock(&symt);

    SymTabRemoveLocalBlock(&symt); // odstránenie lokálneho bloku s parametrami

    fn->init = true; // funkcia je odteraz definovaná
    parser_inside_fn_def = code_inside_fn_def;
    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: RETURN
 *  - po volaní:    NULL
 *
 * @brief Pravidlo pre spracovanie vrátenia návratovej hodnoty funkcie - return
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseReturn() {
    if (!parser_inside_fn_def) { // return sa nachádza v hlavnom tele programu
        logErrSemantic(tkn, "return outside function definition");
        return SEM_ERR_OTHER;
    }

    TRY_OR_EXIT(nextToken());
    char result_type = SYM_TYPE_UNKNOWN;
    switch (tkn->type)
    {
    case ID:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
    case NIL:
    case BRT_RND_L:
        // spracovanie výrazu a zistenie typu návratovej hodnoty v return
        TRY_OR_EXIT(parseExpression(&result_type));
        break;
    default:
        // void return
        saveToken();
        result_type = SYM_TYPE_VOID;
        break;
    }

    TSData_T* fn = SymTabLookupGlobal(&symt, StrRead(&fn_name)); // získanie informácii o predpise aktuálnej funkcie

    if (fn->sig->ret_type == SYM_TYPE_VOID) { // vo vnútri void funkcie
        if (result_type != SYM_TYPE_VOID) { // void funkcia nesmie vraciať hodnotu
            logErrSemanticFn(fn->id, "void function returns a value");
            return SEM_ERR_RETURN;
        }
    }
    else { // funkcia má vraciať hodnotu
        if (result_type == SYM_TYPE_VOID) { // return nič nevracia
            logErrSemanticFn(fn->id, "void return in non-void function");
            return SEM_ERR_RETURN;
        }
        else if (!isCompatibleAssign(fn->sig->ret_type, result_type)) { // návratový typ nesedí s predpisom funkcie
            logErrSemanticFn(fn->id, "different return type");
            return SEM_ERR_FUNC;
        }
    }

    SymTabModifyLocalReturn(&symt, true); // zapísať informáciu o prítomnosti return v aktuálnom bloku

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: IF
 *  - po volaní:    BRT_CUR_R
 *
 * @brief Pravidlo pre spracovanie podmieneného bloku kódu
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseIf() {
    // <STAT>  ->  if <COND> { <PROG> } else { <PROG> }
    TRY_OR_EXIT(nextToken());
    TSData_T* let_variable = NULL; // informácie o premennej v podmienke "let <premenná>"
    switch (tkn->type) // rozlíšenie obyčajnej podmienky v tvare výrazu alebo test premennej na nil "let <premenná>"
    {
    case LET:
        //  <COND> ->  let id
        TRY_OR_EXIT(nextToken());   // identifikátor testovanej premennej
        if (tkn->type != ID) {
            logErrSyntax(tkn, "identifier");
            return SYN_ERR;
        }
        TSData_T* variable = SymTabLookup(&symt, StrRead(&(tkn->atr))); // informácie o premennej
        if (variable == NULL) { // premenná nebola deklarovaná
            logErrSemantic(tkn, "%s was undeclared", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
        if (variable->type == SYM_TYPE_FUNC) { // identifikátor označuje funkciu
            logErrSemantic(tkn, "%s is a function", StrRead(&(tkn->atr)));
            return SEM_ERR_RETURN;
        }
        if (!(variable->init)) { // premenná nebola inicializovaná
            logErrSemantic(tkn, "%s was uninitialized", StrRead(&(tkn->atr)));
            return SEM_ERR_UNDEF;
        }
        if (!(variable->let)) { // premenná musí byť nemodifikovateľná
            logErrSemantic(tkn, "%s must be unmodifiable variable", StrRead(&(tkn->atr)));
            return SEM_ERR_OTHER;
        }

        // premenná musí byť v samostatnom bloku, kde bude jej typ zmenený na typ nezahrňujúci nil
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
        let_variable->type = convertNilTypeToNonNil(variable->type); // ??? treba test či môže obsahovať nil hodnotu if ()
        StrFillWith(&(let_variable->codename), StrRead(&(variable->codename)));

        if (!SymTabInsertLocal(&symt, let_variable)) return COMPILER_ERROR;
        // TODO
        break;
    case ID:    // v podminke je obyčajný výraz
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
    bool if_had_return;
    TRY_OR_EXIT(parseStatBlock(&if_had_return)); // spracovanie príkazov keď podmienka je true
    if (let_variable != NULL) SymTabRemoveLocalBlock(&symt);

    TRY_OR_EXIT(nextToken());
    if (tkn->type != ELSE) {
        logErrSyntax(tkn, "else");
        return SYN_ERR;
    }

    TRY_OR_EXIT(nextToken());
    bool else_had_return;
    TRY_OR_EXIT(parseStatBlock(&else_had_return)); // spracovanie príkazov keď podmienka je false

    if (if_had_return && else_had_return) {
        // pokiaľ sa v oboch častiach if aj else nachádzal return, potom bude return určite zastihnutý
        SymTabModifyLocalReturn(&symt, true);
    }

    return COMPILATION_OK;
}

/**
 * Stav tkn:
 *  - pred volaním: WHILE
 *  - po volaní:    BRT_CUR_R
 *
 * @brief Pravidlo pre spracovanie cyklu while
 * @return 0 v prípade úspechu, inak číslo chyby
*/
int parseWhile() {
    // <STAT>      ->  while exp { <PROG> }
    bool loop_inside_loop = parser_inside_loop; // cyklus v cykle
    if (!loop_inside_loop) {
        StrFillWith(&first_loop_label, "WHILE"); // !!! TODO
    }
    parser_inside_loop = true;

    TRY_OR_EXIT(nextToken());
    switch (tkn->type) // syntaktická kontrola, či sa v podmienke nachádza výraz
    {
    case ID:
    case BRT_RND_L:
    case INT_CONST:
    case DOUBLE_CONST:
    case STRING_CONST:
    case NIL:
        // začiatok výrazu potvrdený
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
    TRY_OR_EXIT(parseStatBlock(NULL));

    parser_inside_loop = loop_inside_loop;
    if (!parser_inside_loop) { // najvrchnejší cyklus bol opustený
        // inštrukcie pre definície premenných vo vnútri cyklu musia byť vložené pred samotným cyklom
        DLLstr_Dispose(&variables_declared_inside_loop);
        // gen code
    }
    return COMPILATION_OK;
}

/* --- FUNKCIE DEKLAROVANÉ V PARSER.H --- */

bool isCompatibleAssign(char dest, char src) {
    if (dest == SYM_TYPE_UNKNOWN) return true;
    if (src == SYM_TYPE_VOID) return false; // nemožno priradiť nič
    if (src == SYM_TYPE_UNKNOWN) return true;
    if (src == SYM_TYPE_NIL) {
        switch (dest) // samotný nil je možné priradiť len do dátového typu zahrňujúceho nil
        {
        case SYM_TYPE_INT_NIL:
        case SYM_TYPE_DOUBLE_NIL:
        case SYM_TYPE_STRING_NIL:
            return true;
        default:
            return false;
        }
    }
    switch (src) // dátový typ nezahrňujúci nil je možné dosadiť aj do typu zahrňujúci nil
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

    loadBuiltInFunctionSignatures();

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
                return SEM_ERR_UNDEF;
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
            variable->init = true;
            destroyToken(first_tkn);
        }
        else {
            logErrSyntax(tkn, "'(' or '='");
            return SYN_ERR;
        }
        break;
    case BRT_CUR_L:
        // <STAT>  ->  { <PROG> }
        bool block_had_return;
        TRY_OR_EXIT(parseStatBlock(&block_had_return));
        if (block_had_return) SymTabModifyLocalReturn(&symt, block_had_return);
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

int checkIfAllFnDef() {
    DLLstr_First(&check_def_fns);
    while (DLLstr_IsActive(&check_def_fns))
    {
        DLLstr_GetValue(&check_def_fns, &fn_name);
        TSData_T* fn_info = SymTabLookupGlobal(&symt, StrRead(&fn_name));
        if (!fn_info->init) {
            logErrSemanticFn(StrRead(&fn_name), "was not defined");
            return SEM_ERR_REDEF;
        }
        DLLstr_Next(&check_def_fns);
    }
    return COMPILATION_OK;
}

void destroyParser() {
    if (tkn != NULL) {
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
