/** Projekt IFJ2023
 * @file main.c
 * @brief Hlavné telo prekladača
 * @author
 * @date 03.10.2023
 */

#include "parser.h"

#define TRY_OR_END_COMPILER(operation)  \
    do {                                \
        int err_code = (operation);     \
        if(err_code != COMPILATION_OK) {\
            destroyParser();            \
            return err_code;            \
        }                               \
    } while (0)


int main()
{
    if (!initializeParser()) return COMPILER_ERROR;
    TRY_OR_END_COMPILER(nextToken());
    while (tkn->type != EOF_TKN)
    {
        TRY_OR_END_COMPILER(parse());
        TRY_OR_END_COMPILER(nextToken());
    }

    destroyParser();

    return COMPILATION_OK;
}