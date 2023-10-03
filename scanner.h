/** Projekt IFJ2023
 * @file scanner.h
 * @brief Lexikálny analyzátor - skener tokenov
 * @author Michal Krulich (xkruli03)
 * @date 03.10.2023
 */

#ifndef _SCANNER_H_
#define _SCANNER_H_

/**
 * @brief ID tokenov
 */
enum token_ids
{
    INVALID,         ///< Neidentifikovaný token
    ID,              ///< Identifikátor
    INT_TYPE,        ///< Typ Int
    DOUBLE_TYPE,     ///< Typ Double
    STRING_TYPE,     ///< Typ String
    INT_NIL_YPE,     ///< Typ Int?
    DOUBLE_NIL_TYPE, ///< Typ Double?
    STRING_NIL_TYPE, ///< Typ String?
    INT_CONST,       ///< Konštanta - Celé číslo
    DOUBLE_CONST,    ///< Konštanta - Desatinné číslo
    STRING_CONST,    ///< Konštanta - Reťazec
    VAR,             ///< var
    LET,             ///< let
    IF,              ///< if
    ELSE,            ///< else
    WHILE,           ///< while
    FUNC,            ///< func
    RETURN,          ///< return
    NIL,             ///< nil
    UNDERSCORE,      ///< podtržítko _
    BRT_RND_L,       ///< bracket round left (
    BRT_RND_R,       ///< bracket round left )
    BRT_CUR_L,       ///< bracket curly left {
    BRT_CUR_R,       ///< bracket curly left }
    OP_PLUS,         ///< operátor +
    OP_MINUS,        ///< operátor -
    OP_MUL,          ///< operátor *
    OP_DIV,          ///< operátor /
    SEMICOLON,       ///< bodkočiarka ;
    COMMA,           ///< čiarka ,
    COLON,           ///< dvojbodka :
    EOF_TKN          ///< Koniec súboru
};

/**
 * @brief Token
 * @warning atr treba potom neskôr nahradiť so str_T dátovým typom
 */
typedef struct token
{
    int type;      ///< typ tokenu
    char atr[100]; ///< atribut tokenu, prečítaný reťazec
    int ln;        ///< riadok tokenu
    int col;       ///< pozícia prvého charakteru tokenu v riadku
} token_T;

/**
 * @brief Vráti uschovaný token, inak prečíta ďalší token na vstupe STDIN
 * @return ukazateľ na alokovaný token
 * @details Preskočí prvých n bielych znakov, alokuje pamäť pre dátovú štruktúru token
 * a naplní ju prečítanými hodnotami:
 *  - type:     rozsah token_ids
 *  - atr:      náazov identifikátora, hodnota konštanty ako reťazec, v ostatných prípadoch nedefinované
 *  - ln:       riadok, ktorým začínal token
 *  - col:      stĺpec, ktorým začínal token
 */
token_T *getToken();

/**
 * @brief Uloží token do pamäte skenera
 * @details Hodnota je uschovaná v globálnej premennej. NULL značí, že pamäť je prázdna.
 */
void storeToken(token_T *tkn);

#endif // ifndef _SCANNER_H_
/* Koniec súboru scanner.h */
