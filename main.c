/** Projekt IFJ2023
 * @file main.c
 * @brief Hlavné telo prekladača
 * @author Michal Krulich (xkruli03)
 * @date 17.11.2023
 */

#include "parser.h"

int main() {
    if (!initializeParser()) return COMPILER_ERROR; // inicializácia dátových štruktúr parsera
    TRY_OR_EXIT(nextToken()); // načítať prvý token
    while (tkn->type != EOF_TKN)
    {
        TRY_OR_EXIT(parse());   // spracovanie základneho príkazu, pravidlo <STAT>
        TRY_OR_EXIT(nextToken());
    }

    TRY_OR_EXIT(checkIfAllFnDef()); // zistí, či boli definované všetky volané funkcie

    printOutCompiledCode(); // výpis vygenerovaného cieľového kódu

    destroyParser(); // dealokácia použitých zdrojov

    return COMPILATION_OK;
}