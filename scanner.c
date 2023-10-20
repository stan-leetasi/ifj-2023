/**
 * @file scanner.c
 * @author František Holáň (xholan13@stud.fit.vut.cz)
 * @brief Lexikální analyzátor
 * @version 0.1
 * @date 2023-10-15
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "scanner.h"

#define END_OF_MULITINE_STRING 3 //Počet uvozovek které jsou třeba k uknčení víceřádkového řetězce
#define NUM_OF_KEYWORDS 11       //Počet klíčových slov v tabulce klíčových slov

/*Tabulka klíčových slov*/
static const char *key_words[] = {
    "Double", 
    "else",
    "func",
    "if",
    "Int",
    "let",
    "var",
    "nil",
    "return",
    "String",
    "while"
};

//stavy automatu
typedef enum state {
    INIT_STATE_S, 
    COMMENT_SLASH_S, 
    COMMENT_BLOCK_S,
    COMMENT_BLOCK_END_S,
    COMMENT_LINE_S,
    DASH_MINUS_S,
    EQ_S, 
    GT_S, 
    LT_S, 
    STRING_S,
    EXCL_S,
    ID_S, 
    UNDERSCORE_S,
    INT_NUMBER_S,
    DOUBLE_NUMBER_S,
    EXP_NUMBER_S,
    EXP_NUMBER_SIGN_S,
    PRE_DOUBLE_NUMBER_S,
    SINGLE_LINE_STRING_S,
    MULTI_LINE_STRING_S,
    PRE_MULTI_LINE_STRING_S,
    MULTI_LINE_STRING_END_S,
    EMPTY_STRING_S,
    ESCAPE_SEKV_S,
    QUEST_MARK_S
} state_t;

//Globální proměnná reprezentující vstupní soubor
FILE *source;

///< úschovňa pre jeden token
token_T *storage = NULL; 

/**
 * @brief Načte do globální proměnné "source" vstupní soubor
 * 
 * @param f Vstupní soubor
 */
void setSourceFile(FILE *f)
{
  source = f;
}

/**
 * @brief Zjistí, jestli se v tabulce klíčových slov nachází konkrétní řetězec
 * 
 * @param key_words Tabulka klíčových slov
 * @param word Hledaný řetězec
 * @param size velikost tabulky klíčových slov
 * @return true 
 * @return false 
 */
bool find_key_word(const char ** const key_words, const char * word, unsigned size) {
    for (unsigned i = 0; i < size; i++) {
        //řetězce se rovnají
        if (strcmp(key_words[i], word) == 0) {
            return true;
        } 
    }
    return false;
}

/**
 * @brief Převede řetězec, který odpovídá klíčovému slovu na odpovídající token id
 * 
 * @param string 
 * @return int, 0 pokud string neodpovídá žádnému klíčovému slovu
 */
int keyw_token_num(char * string) {
    if (strcmp(string, "Double") == 0) {
        return DOUBLE_TYPE;
    } else if (strcmp(string, "else") == 0) {
        return ELSE;
    } else if (strcmp(string, "func") == 0) {
        return FUNC;
    } else if (strcmp(string, "if") == 0) {
        return IF;
    } else if (strcmp(string, "Int") == 0) {
        return INT_TYPE;
    } else if (strcmp(string, "let") == 0) {
        return LET;
    } else if (strcmp(string, "var") == 0) {
        return VAR;
    } else if (strcmp(string, "nil") == 0) {
        return NIL;
    } else if (strcmp(string, "return") == 0) {
        return RETURN;
    } else if (strcmp(string, "String") == 0) {
        return STRING_TYPE;
    } else if (strcmp(string, "while") == 0) {
        return WHILE;
    }
    return 0;
}

/**
 * @brief Inicializuje strukturu token
 * 
 * @param token token
 * @return ukazatel na token 
 */
token_T* init_token(token_T **token) {
    //alokace pameti pro token
    *token = malloc(sizeof(token_T));

    //alokace paměti pro token se nepovedla, vrať false
    if (*token == NULL) {
        return NULL;
    } 

    //Inicializace struktury
    (*token)->type = INVALID;
    (*token)->atr = NULL;
    (*token)->ln = 0;
    (*token)->col = 0;

    return *token;
}

/**
 * @brief Nastaví token strukturu
 * 
 * @param type 
 * @param atr 
 * @param ln 
 * @param col_begin_token 
 * @return token_T*
 */
token_T* set_token(int type, str_T *atr, int ln, int col) {
    storage->type = type;
    storage->atr = atr;
    storage->ln = ln;
    storage->col = col;
    return storage;
}

/**
 * @brief Funkce funguje jako podautomat. Analyzuje escape sekvence v řetězci
 * 
 * @param c 
 * @return int 
 */
int escape_seq_process(char c) {
    static int state = 0; //Stav -- stavy jsou celkem tři {0,1,2}
    static int num_of_digits = 0; //Zde bude uložen počet hexadecimálních číslic (může jich být maximálně 8)
    int result = 0; //Výsledek procesu, 0 = false, 1 = true, -1 = je v procesu
    switch (state)
    {
    case 0: //jednoduché (jednoznakové) escape sekvence
        switch (c) {
            case '"':
            case '\\':
            case 'n':
            case 'r':
            case 't': 
                result = 1; //Vše je v pořádku vrací se true
                break;
            case 'u':
                //unicode escape sekvence
                state = 1;
                result = -1;
                break;
            default:
                //Na vstupu je špatný znak
                result = 0;
                break;
        }
        break;
    case 1: //na vstupu je levá složená závorka
        if (c == '{') {
            state = 2;
            result = -1;
        } else { 
            result = 0;  
        }
        break;
    case 2: 
        if (isxdigit(c) && num_of_digits < 8) {
        //Na vstupu může být jakékoliv hexadecimální číslo, ale může jich být pouze 8
            state = 2;
            result = -1;
            num_of_digits++;
        } else if (c == '}' && num_of_digits >= 1) {
        //Ukončení této sekvence pomocí levé složené závorky
        //Uvnitř složených závorek musí být alespoň jedna hexadecimální číslice
            result = 1;
        } else {
        //Na vstupu je špatný znak
            result = 0;
        }
        break;
    }
    return result;
}


token_T *getToken()
{
    //stav automatu
    state_t state = 0;
    //čtený znak
    int c;

    //Lokální statické proměnné pro řádek a sloupec začínajícího tokenu
    static int ln = 1;          //řádek
    static int col = 0;         //sloupec

    //Pomocné proměnné pro některé stavy
    int col_begin_token = 0;    //speciální proměnná pro uložení pozice, kde začíná víceznakový token (string, identifikátor,...)
    int quote_mark_num = 0;     //speciální proměnná, která indikuje počet za sebou jdoucích uvozovek
    bool is_multi_line_string = 0;  //speciální proměnná, která indikuje zda se přešlo do stavu ESCAPE_SEKV_S z víceřádkového řetězce nebo z jednořádkového řetězce

    //inicializace řetězce, kam se budou ukládat víceznakové tokeny
    str_T str; 
    str_T *string = &str;
    StrInit(string);
    //inicializace struktury
    if (init_token(&storage) == NULL) {
        return NULL;
    }
    
/*===============================================================HLAVNÍ SMYČKA===============================================================*/
    while(true) {
        //načtení znaku ze souboru
        c = fgetc(source);

        //přečtený další znak na jednom řádku
        col++;
        
        //Znak nového řádku, je třeba inkrementovat ln = line number
        if (c == '\n') {
            ln++;
            col = 0;
        }

        switch (state)
        {
/*=======================================STATE=======================================*/
            case INIT_STATE_S:
                
                col_begin_token = col;

                if (isspace(c)) {
                    //Na vstupu jsou bílé znaky, které je třeba přeskočit
                    //Ignorujeme
                    state = INIT_STATE_S;
                } else if (c == '/') {
                    //komentář - bude se ignorovat
                    state = COMMENT_SLASH_S;
                } else if (isdigit(c)) {
                    //číslo
                    state = INT_NUMBER_S;
                } else if (c == '"') {
                    //Řetězec
                    state = STRING_S;
                } else if (isalpha(c)) {
                    //Identifikátor nebo klíčové slovo
                    state = ID_S;
                } else if (c == '-') {
                    //Minus/pomlčka
                    state = DASH_MINUS_S;
                }
                //operátory plus a krát
                else if (c == '+') return set_token(OP_PLUS, NULL, ln, col_begin_token);
                else if (c == '*') return set_token(OP_MUL, NULL, ln, col_begin_token);
                //dvojtečka
                else if (c == ':') return set_token(COLON, NULL, ln, col_begin_token);
                //čárka
                else if (c == ',') return set_token(COMMA, NULL, ln, col_begin_token);
                else if (c == '!') {
                    //vykřičník
                    state = EXCL_S;   
                } else if (c == '_') {
                    //podtržítko
                    state = UNDERSCORE_S;
                } else if (c == '=') {
                    //rovnítko
                    state = EQ_S;
                } else if (c == '>') {
                    //Znak "větší"
                    state = GT_S;
                } else if (c == '<') {
                    //Znak "menší"
                    state = LT_S;
                } else if (c == '?') {
                    //Otazník
                    state = QUEST_MARK_S;
                }
                //Závorky 
                else if (c == '(') return set_token(BRT_RND_L, NULL, ln, col_begin_token);
                else if (c == ')') return set_token(BRT_RND_R, NULL, ln, col_begin_token);
                else if (c == '{') return set_token(BRT_CUR_L, NULL, ln, col_begin_token);
                else if (c == '}') return set_token(BRT_CUR_R, NULL, ln, col_begin_token);
                //Konec souboru
                else if (c == EOF) return set_token(EOF_TKN, NULL, ln, col_begin_token);
                //není přečtený správný znak
                else 
                    return set_token(INVALID, NULL, ln, col_begin_token);
                //Přidání znaku do řetězce (jde o víceznakový token)   
                if (!isspace(c) && c != '/') {
                    StrAppend(string, c);
                }
                break;
/*=======================================STATE=======================================*/
            case COMMENT_SLASH_S:
                switch(c) {
                    //blokový komentář
                    case '*':
                        state = COMMENT_BLOCK_S;
                        break;
                    //řádkový komentář
                    case '/':
                        state = COMMENT_LINE_S;
                        break;
                    //na vstupu není ani * ani / => jedná se o operátor děleno
                    default:
                        //potřeba vrátit znak do STREAMu
                        ungetc(c, source);
                        col--;
                        return set_token(OP_DIV, NULL, ln, col_begin_token);
                        break;
                }
                break;
/*=======================================STATE=======================================*/               
            case COMMENT_BLOCK_S:
                if (c != '*' && c != EOF) {
                    //'/*' ==> blokový komentář, zůstaň v tomto stavu
                    state = COMMENT_BLOCK_S;
                } else if (c == EOF) {
                    //Neukončený blokový komentář
                    return set_token(INVALID, NULL, ln, col_begin_token);
                } else {
                    state = COMMENT_BLOCK_END_S;
                }
                break;
/*=======================================STATE=======================================*/ 
            case COMMENT_BLOCK_END_S:
                if (c == '/') {
                    //blokový komentář je ukončen, vrať se do začátečního stavu stavu
                    state = INIT_STATE_S;
                } else if (c == EOF) {
                    //Neukončený blokový komentář
                    return set_token(INVALID, NULL, ln, col_begin_token);  
                } else {
                    state = COMMENT_BLOCK_S;
                }
                break;
/*=======================================STATE=======================================*/ 
            case COMMENT_LINE_S:
                if (c == '\n') {
                    //řádkový komentář zahrnuje vše až do konce řádku
                    state = INIT_STATE_S;
                } else if (c == EOF) {
                    //Přečtený EOF
                    col--;
                    ungetc(c, source);
                    state = INIT_STATE_S;  
                } else {
                    state = COMMENT_LINE_S;
                }
                break;
/*=======================================STATE=======================================*/
            case INT_NUMBER_S:
                if (isdigit(c)) {
                    //Na vstupu jsou čísla = zůstává se ve stavu celé číslo
                    state = INT_NUMBER_S;
                } else if (tolower(c) == 'e') {
                    //celé číslo s exponentem
                    state = EXP_NUMBER_S;
                } else if (c == '.') {
                    //desetinné číslo
                    state = PRE_DOUBLE_NUMBER_S;
                } else {
                    ungetc(c, source);
                    col--;
                    /*POZOR JE TREBA OPRAVIT PREDAT UZ RETEZEC (MISTO NULL)*/
                    /*col_begin_token musi byt podle prvni pozice cisla*/
                    return set_token(INT_CONST, string, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case PRE_DOUBLE_NUMBER_S:
                if (isdigit(c)) {
                    //desetinné číslo s tečkou
                    state = DOUBLE_NUMBER_S;
                } else {
                    //na vstup nepřišel žádný vhodný znak
                    return set_token(INVALID, NULL, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case DOUBLE_NUMBER_S:
                if (isdigit(c)) {
                    //zůstaň v tomto stavu (na vstupu jsou čísla)
                    state = DOUBLE_NUMBER_S;
                } else if (tolower(c) == 'e') {
                    //desetinné číslo s exponentem
                    state = EXP_NUMBER_S;
                } else {
                    /*col_begin_token musi byt podle prvni pozice cisla*/
                    return set_token(DOUBLE_CONST, string, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case EXP_NUMBER_S:
                if (isdigit(c)) {
                    state = DOUBLE_NUMBER_S;
                } else if (c == '+' || c == '-') {
                    //exponenciální tvar s volitelným znaménkem
                    state = EXP_NUMBER_SIGN_S;
                } else {
                    //špatná hodnota
                    return set_token(INVALID, NULL, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case EXP_NUMBER_SIGN_S:

                if (isdigit(c)) {
                    state = DOUBLE_NUMBER_S;
                } else {
                    return set_token(INVALID, NULL, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case ID_S:
                //Součástí názvu identifikátoru může být jakýkoliv alfanumerický znak a podtržítko
                if (isalnum(c) || c == '_') {
                    state = ID_S;
                } else {
                    /*UPRAVIT NULL NA REALNY RETEZEC, UPRAVIT MAGICKOU KONSTANTU*/
                    col--;
                    ungetc(c, source);
                    if (find_key_word(key_words, string->data, NUM_OF_KEYWORDS)) {
                        //Bylo nalezeno klíčové slovo, vrátí se token konkrétního klíčového slova
                        return set_token(keyw_token_num(string->data), string, ln, col_begin_token);
                    } else {
                        /*LN a col_begin_token musi byt nastaveno od zacatku identifikatoru*/
                        return set_token(ID, string, ln, col_begin_token);
                    }
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case UNDERSCORE_S:
                if (isalnum(c)) {
                    //Bude se jednat o identifikátor začínající podtržítkem
                    state = ID_S;
                } else {
                    //jedná se o znak podtržítko
                    ungetc(c, source);
                    col--;
                    return set_token(UNDERSCORE, NULL, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case STRING_S:
                if (c == '"') {
                    //prázdný řetězec
                    state = EMPTY_STRING_S;
                } else if (c == EOF) {
                    //Otevřený řetězec, za kterým následuje EOF
                    col--;
                    ungetc(c, source);
                    return set_token(INVALID, NULL, ln, col_begin_token);
                } else {
                    //jednoduchý řetězec
                    state = SINGLE_LINE_STRING_S;
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case SINGLE_LINE_STRING_S:
                if (c == '"') {
                    //konec jednořádkového řetězce
                    return set_token(STRING_CONST, string, ln, col_begin_token);
                } else if (c == EOF) {
                  //Neukončený řetězec
                    ungetc(c, source);
                    col--;
                    return set_token(INVALID, NULL, ln, col_begin_token);  
                } else if (c == '\\') {
                    //Escape sekvence
                    is_multi_line_string = false;
                    state = ESCAPE_SEKV_S;
                } else {
                    state = SINGLE_LINE_STRING_S;
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case EMPTY_STRING_S:
                //Toto je přechodový stav mezi víceřádkovým řetězcem a jednořádkovým řetězcem

                if (c == '"') {
                    state = PRE_MULTI_LINE_STRING_S;
                } else {
                    ungetc(c, source);
                    col--;
                    return set_token(STRING_CONST, string, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case PRE_MULTI_LINE_STRING_S:
                //Tři uvozovky (víceřádkový řetězec) musí být na samostatném řádku
                if (c == '\n') {
                    state = MULTI_LINE_STRING_S;
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                } else if (isblank(c)) {
                    //Bílé znaky se ignorují
                    //Nebudou se do řetězce přidávat
                    state = PRE_MULTI_LINE_STRING_S;
                } else {
                    return set_token(INVALID, NULL, ln, col_begin_token);
                }
                break;
/*=======================================STATE=======================================*/
            case MULTI_LINE_STRING_S:
                //Víceřádkový řetězec
                if (c == '\n') {
                    //Potenciální možnost ukončení řetězce
                    state = MULTI_LINE_STRING_END_S;
                } else if (c == EOF) {
                    ungetc(c, source);
                    col--;
                    return set_token(INVALID, NULL, ln, col_begin_token);
                } else if (c == '\\') {
                    //Escape sekvence
                    is_multi_line_string = true;
                    state = ESCAPE_SEKV_S;
                } else {
                    //Na vstup přichází správné znaky, zůstaň v tomto stavu
                    state = MULTI_LINE_STRING_S;
                }

                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case MULTI_LINE_STRING_END_S:
                //Řetězec může být ukončen
                if (c == '"') {
                    quote_mark_num++;
                    //Abych nemusel přecházet do dalších dvou stavů, je zde pomocná proměnná, která počítá uvozovky
                    if (quote_mark_num == END_OF_MULITINE_STRING) {
                        return set_token(STRING_CONST, string, ln, col_begin_token);
                    }
                    state = MULTI_LINE_STRING_END_S;

                } else if (c == EOF) {
                    //Neukončený řetězec
                    return set_token(INVALID, NULL, ln, col_begin_token);
                } else {
                    quote_mark_num = 0;
                    state = MULTI_LINE_STRING_S;
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case ESCAPE_SEKV_S:
                //Zde se zpracovává escape sekvence v řetězci
                if (escape_seq_process(c) == -1) {
                    //analýza escape sekvence není u konce, zůstává se v tomto stavu
                    state = ESCAPE_SEKV_S;
                } else if (escape_seq_process(c)) {
                    //analýza escape sekvence proběhla úspěšně
                    if (is_multi_line_string) {
                        state = MULTI_LINE_STRING_S;
                    } else {
                        state = SINGLE_LINE_STRING_S;
                    }
                } else if (escape_seq_process(c) == 0) {
                    //Analýza escape sekvence neproběhla úspěšně
                    return set_token(INVALID, NULL, ln, col_begin_token);
                }
                //Přidání znaku na konec řetězce
                StrAppend(string, c);
                break;
/*=======================================STATE=======================================*/
            case DASH_MINUS_S:
                
                //Přečtený znak je '>': výsledný token: ->
                if (c == '>') {
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                    return set_token(ARROW, string, ln, col_begin_token);
                } else {
                    //Jedná se o pomlčku = operátor minus (jednoznakový operátor)
                    ungetc(c, source);
                    col--;
                    return set_token(OP_MINUS, NULL, ln, col_begin_token);
                } 
                break;
/*=======================================STATE=======================================*/
            case EQ_S:
                //přiřazení nebo porovnání
                if (c == '=') {
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                    return set_token(EQ, string, ln, col_begin_token);
                } else {
                    //Operátor přiřazení
                    ungetc(c, source);
                    col--;
                    return set_token(ASSIGN, NULL, ln, col_begin_token);
                }
                break;
/*=======================================STATE=======================================*/
            case GT_S:

                //větší nebo větší rovno
                if (c == '=') {
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                    return set_token(GTEQ, string, ln, col_begin_token);
                } else {
                    ungetc(c, source);
                    col--;
                    return set_token(GT, NULL, ln, col_begin_token);
                }
                break;
/*=======================================STATE=======================================*/
            case LT_S:

                //menší nebo menší rovno
                if (c == '=') {
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                    return set_token(LTEQ, string, ln, col_begin_token);
                } else {
                    //Operátor menší
                    ungetc(c, source);
                    col--;
                    return set_token(LT, NULL, ln, col_begin_token);
                }
                break;
/*=======================================STATE=======================================*/
            case EXCL_S:

                if (c == '=') {
                    //Negace porovnání
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                    return set_token(NEQ, string, ln, col_begin_token);
                } else {
                    ungetc(c, source);
                    col--;
                    return set_token(EXCL, NULL, ln, col_begin_token);
                }
                break;
/*=======================================STATE=======================================*/
            case QUEST_MARK_S:

                if (c == '?') {
                    //Operátor ??
                    //Přidání znaku na konec řetězce
                    StrAppend(string, c);
                    return set_token(TEST_NIL, string, ln, col_begin_token);
                } else {
                    ungetc(c, source);
                    col--;
                    return set_token(QUEST_MARK, NULL, ln, col_begin_token);
                }
                break;
        }
    }
    return NULL;
}

void storeToken(token_T *tkn)
{

}
