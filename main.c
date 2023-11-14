/** Projekt IFJ2023
 * @file main.c
 * @brief Hlavné telo prekladača
 * @author
 * @date 03.10.2023
 */

#include "parser.h"

int main()
{
    if(!initializeParser()) return COMPILER_ERROR;
    TRY_OR_EXIT(nextToken());
    if (tkn == NULL) return COMPILER_ERROR;
    if (tkn->type == INVALID) return LEX_ERR;
    while (tkn->type != EOF_TKN)
    {
        TRY_OR_EXIT(parse());
        TRY_OR_EXIT(nextToken());
    }
    
    return COMPILATION_OK;
}