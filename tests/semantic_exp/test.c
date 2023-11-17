#include <stdio.h>
#include <assert.h>
#include "../../parser.h"
#include "../../exp.h"

void create_variable_info(char* id, char var_type, bool init) {
    TSData_T* var = SymTabCreateElement(id);
    var->type = var_type;
    var->init = init;

    if(var_type == SYM_TYPE_FUNC) {
        var->sig = SymTabCreateFuncSig();
    }

    SymTabInsertLocal(&symt, var);
}

void init_used_variables() {
    create_variable_info("a", SYM_TYPE_INT, true);
    create_variable_info("b", SYM_TYPE_INT, true);
    create_variable_info("c", SYM_TYPE_INT, true);
    create_variable_info("d", SYM_TYPE_INT, true);

    create_variable_info("m", SYM_TYPE_DOUBLE, true);
    create_variable_info("n", SYM_TYPE_DOUBLE, true);
    create_variable_info("o", SYM_TYPE_DOUBLE, true);
    create_variable_info("p", SYM_TYPE_DOUBLE, true);

    create_variable_info("text", SYM_TYPE_STRING, true);

    create_variable_info("A_nil", SYM_TYPE_INT_NIL, true);
    create_variable_info("M_nil", SYM_TYPE_DOUBLE_NIL, true);
    
    create_variable_info("fun_fn", SYM_TYPE_FUNC, true);

    create_variable_info("uninit", SYM_TYPE_DOUBLE, false);
}

int main() {
    char expected_type = getchar(); // na prvom riadku sample-u je očakávaný typ výsledku výrazu

    initializeParser();
    TRY_OR_EXIT(nextToken());

    char result_type = SYM_TYPE_VOID;
    int ret_code = parseExpression(&result_type);

    destroyParser();

    // X na začiatku sample znamená, že test neskúša návratový typ výrazu
    if (expected_type != 'X') {
        if (expected_type != result_type) {
            fprintf(stderr, "test.c: INCORRECT RESULT TYPE - expected type '%c', but got '%c'\n", expected_type, result_type);
            return 50;
        }
    }

    if(tkn != NULL) {
        fprintf(stderr, "test.c: tkn must remain NULL after parseExpression\n");
        return 50;
    }

    return ret_code;
}