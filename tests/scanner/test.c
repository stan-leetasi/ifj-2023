#include "../../scanner.h"
#include <stdio.h>
#include <stdlib.h>

char *tkn_ids[] = {
    "INVALID",         ///< Neidentifikovaný token
    "ID",              ///< Identifikátor
    "INT_TYPE",        ///< Typ Int
    "DOUBLE_TYPE",     ///< Typ Double
    "STRING_TYPE",     ///< Typ String
    "INT_NIL_YPE",     ///< Typ Int?
    "DOUBLE_NIL_TYPE", ///< Typ Double?
    "STRING_NIL_TYPE", ///< Typ String?
    "INT_CONST",       ///< Konštanta - Celé číslo
    "DOUBLE_CONST",    ///< Konštanta - Desatinné číslo
    "STRING_CONST",    ///< Konštanta - Reťazec
    "VAR",             ///< var
    "LET",             ///< let
    "IF",              ///< if
    "ELSE",            ///< else
    "WHILE",           ///< while
    "FUNC",            ///< func
    "RETURN",          ///< return
    "NIL",             ///< nil
    "UNDERSCORE",      ///< podtržítko _
    "BRT_RND_L",       ///< bracket round left (
    "BRT_RND_R",       ///< bracket round left )
    "BRT_CUR_L",       ///< bracket curly left {
    "BRT_CUR_R",       ///< bracket curly left }
    "OP_PLUS",         ///< operátor +
    "OP_MINUS",        ///< operátor -
    "OP_MUL",          ///< operátor *
    "OP_DIV",          ///< operátor /
    "SEMICOLON",       ///< bodkočiarka ;
    "COMMA",           ///< čiarka ,
    "COLON",           ///< dvojbodka :
    "EOF_TKN"          ///< Koniec súboru
};

int main()
{
    token_T *tkn = NULL;
    while ((tkn = getToken())->type != EOF_TKN)
    {
        if (tkn != NULL)
        {
            printf("%s\n", tkn_ids[tkn->type]);
            free(tkn);
        }
    }
    if(tkn != NULL)
    {
        printf("%s\n", tkn_ids[EOF_TKN]);
        free(tkn);
    }

    return 0;
}