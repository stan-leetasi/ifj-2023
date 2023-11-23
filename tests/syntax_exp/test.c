#include "../../parser.h"
#include "../../exp.h"

void create_variable_info(char* id, char var_type, bool init) {
    TSData_T* var = SymTabCreateElement(id);
    var->type = var_type;
    var->init = init;

    if (var_type == SYM_TYPE_FUNC) {
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
    initializeParser();
    init_used_variables();

    TRY_OR_EXIT(nextToken());

    char result_type;
    bool literal; 
    int ret_code = parseExpression(&result_type, &literal);

    if (tkn != NULL && ret_code == COMPILATION_OK) {
        fprintf(stderr, "test.c: tkn must remain NULL after parseExpression\n");
        return 50;
    }

    destroyParser();
    tkn = getToken();

    return ret_code;
}