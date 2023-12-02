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
    "ARROW",           ///< šípka ->
    "BRT_RND_L",       ///< bracket round left (
    "BRT_RND_R",       ///< bracket round left )
    "BRT_CUR_L",       ///< bracket curly left {
    "BRT_CUR_R",       ///< bracket curly left }
    "OP_PLUS",         ///< operátor +
    "OP_MINUS",        ///< operátor -
    "OP_MUL",          ///< operátor *
    "OP_DIV",          ///< operátor /
    "ASSIGN",          ///< priradenie =
    "EQ",              ///< porovnanie ==
    "NEQ",             ///< negace porovnanie !=  
    "GT",              ///< porovnanie >
    "GTEQ",            ///< porovnanie >=
    "LT",              ///< porovnanie <
    "LTEQ",            ///< porovnanie <=
    "EXCL",            ///< výkričník !
    "QUEST_MARK",      ///< otazník ?   
    "TEST_NIL",        ///< test nil hodnoty ??
    "COMMA",           ///< čiarka ,
    "COLON",           ///< dvojbodka :
    "EOF_TKN"          ///< Koniec súboru
};

int main()
{
    token_T *tkn = NULL;
    tkn = getToken();
    while (tkn->type != EOF_TKN && tkn->type != INVALID)
    {
        if (tkn != NULL)
        {
            printf("%s\n", tkn_ids[tkn->type]);
            destroyToken(tkn);
        }
        tkn = getToken();
    }
    if(tkn != NULL)
    {
        printf("%s\n", tkn_ids[EOF_TKN]);
        destroyToken(tkn);
    }

    return 0;
}