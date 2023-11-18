/** Projekt IFJ2023
 * @file main.c
 * @brief Hlavné telo prekladača
 * @author Michal Krulich (xkruli03)
 * @date 17.11.2023
 */

#include "parser.h"

int main()
{
    if (!initializeParser()) return COMPILER_ERROR;
    TRY_OR_EXIT(nextToken());
    while (tkn->type != EOF_TKN)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }
    TRY_OR_EXIT(checkIfAllFnDef());

    destroyParser();

    return COMPILATION_OK;
}