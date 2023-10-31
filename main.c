/** Projekt IFJ2023
 * @file main.c
 * @brief Hlavné telo prekladača
 * @author
 * @date 03.10.2023
 */

#include "parser.h"

int main()
{
    if(initializeParser()) return COMPILER_ERROR;
    tkn = getToken();
    
    return 0;
}