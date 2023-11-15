/** Projekt IFJ2023
 * @file exp.c
 * @brief Precedenčná syntaktická a sémantická analýza výrazov s generovaním cieľového kódu
 * @author
 * @date
 */

#include "exp.h"
#include "logErr.h"

int parseExpression(char* result_type) {

    /* DUMMY IMPLEMENTATION. FOR TOP-DOWN TESTING PURPOSES */

    bool first_loaded = false;
    bool binary_op_loaded = false;
    bool stop = false;
    while (!stop)
    {
        if (!first_loaded && tkn->type != BRT_RND_L) {
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
                *result_type = variable->type;
                first_loaded = true;
                break;
            case INT_CONST:
                *result_type = SYM_TYPE_INT;
                first_loaded = true;
                break;
            case DOUBLE_CONST:
                *result_type = SYM_TYPE_DOUBLE;
                first_loaded = true;
                break;
            case STRING_CONST:
                *result_type = SYM_TYPE_STRING;
                first_loaded = true;
                break;
            case NIL:
                *result_type = SYM_TYPE_NIL;
                first_loaded = true;
                break;
            default:
                break;
            }
        }
        switch (tkn->type)
        {
        case ID:
        case BRT_RND_L:
        case INT_CONST:
        case DOUBLE_CONST:
        case STRING_CONST:
        case NIL:
        case BRT_RND_R:
            binary_op_loaded = false;
            break;
        case EQ:
        case NEQ:
        case GT:
        case GTEQ:
        case LT:
        case LTEQ:
            *result_type = SYM_TYPE_BOOL;
            binary_op_loaded = true;
            break;
        case OP_PLUS:
        case OP_MINUS:
        case OP_MUL:
        case OP_DIV:
        case TEST_NIL:
            binary_op_loaded = true;
            break;
        default:
            saveToken();
            stop = true;
            break;
        }
        if(!stop) TRY_OR_EXIT(nextToken());
    }

    if(binary_op_loaded) {
        logErrSyntax(tkn, "exp syntax");
        return SYN_ERR;
    }

    /* DUMMY IMPLEMENTATION. FOR TOP-DOWN TESTING PURPOSES */

    return COMPILATION_OK;
}

/* Koniec súboru exp.c */
