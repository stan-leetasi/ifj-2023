/**
 * @file decode.c
 * @author František Holáň (xholan13@stud.fit.vut.cz)
 * @brief 
 * @version 0.1
 * @date 2023-10-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "decode.h"
#include "strR.h"
#include <ctype.h>
#include <stdlib.h>
#include <stdbool.h>

//Délka řetězce, který je ve tvaru \xyz 
#define CHAR_CODE_LEN 5

enum codes {
    SPACE, 
    BACKSLASH, 
    NEW_LINE, 
    HASHTAG,
    CARRIAGE_RETURN,
    TAB,
};

//Stavy automatu
typedef enum state {
    INIT_S,
    BACKSLASH_S,
    UNICODE_S
} state_t;


char *string_codes[] = {
    "\\032",    //mezera
    "\\092",    //zpětné lomítko 
    "\\010",    //nový řádek \n
    "\\035",    //mřížka #
    "\\013",    //přesun kurzoru na začátek \r
    "\\009"     //odsazení \t
};
/**
 * @brief Pomocná funkce, připojí obyčejný řetězec (char *) k řetězci datového typu str_T *
 * 
 * @param dest sem se připojí řetězec
 * @param string připojovaný řetězec
 */
void strAdd(str_T * dest, char * string) {
    str_T str;
    StrInit(&str);
    StrFillWith(&str, string);
    StrCat(dest, &str);
    StrDestroy(&str);
}

str_T strEncode(char * string) {
    //stavy automatu
    state_t state = INIT_S;
    //zde bude uložený nový řetězec
    str_T decoded_string;
    //Inicializace datové struktury str_T
    StrInit(&decoded_string);
    //Iterátor - touto proměnnou se bude iterovat znak po znaku řetězecv "string"
    unsigned i = 0;
    //Zde bude konkrétní znak ze "string"
    int c;
    //Zde je uložené unicode číslo v escape sekvenci v hexadecimálním tvaru
    str_T unicode_number;
    StrInit(&unicode_number);
    //Zde bude uložena sekvence ve tvaru \xyz
    char unicode_result[CHAR_CODE_LEN];
    //Na začátek řetězce přidá prefix string@
    StrFillWith(&decoded_string, "string@");

    //Procházení řetězce "string" znak po znaku
    while((c = string[i]) != '\0') {

        switch (state) {
            case INIT_S: ;
                switch(c) {
                    case ' ': ;
                        //mezera
                        strAdd(&decoded_string, string_codes[SPACE]);
                        break;
                    case '\\': ;
                        //Zpětné lomítko
                        state = BACKSLASH_S;
                        break;
                    case '#': ;
                        //Hashtag
                        strAdd(&decoded_string, string_codes[HASHTAG]);
                        break;
                    case '\n': ;
                        //Byl přečteny novy radek (pro multiline retezce)
                        strAdd(&decoded_string, string_codes[NEW_LINE]);
                        break;
                    default: ;
                        //Cokoli jineho
                        StrAppend(&decoded_string, c);
                        break;
                }
                break;
            case BACKSLASH_S: ;
                state = INIT_S;
                
                switch (c) {
                    case 'n': ;
                        //Novy radek
                        strAdd(&decoded_string, string_codes[NEW_LINE]);
                        break;
                    case '\\': ;
                        strAdd(&decoded_string, string_codes[BACKSLASH]);
                        break;
                    case 't': ;
                        strAdd(&decoded_string, string_codes[TAB]);
                        break;
                    case 'r': ;
                        strAdd(&decoded_string, string_codes[CARRIAGE_RETURN]);
                        break;
                    case '"': ;
                        StrAppend(&decoded_string, c);
                        break;
                     case 'u': ;
                        state = UNICODE_S;
                        break;
                }

                break;
            case UNICODE_S: ;
                //Zpracování unicode sekvence
                if (c == '{') {
                    state = UNICODE_S;
                } else if (isdigit(c) || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F')) { // hexa 0-9 + a-f
                    StrAppend(&unicode_number, c);
                    state = UNICODE_S;
                } else if (c == '}') {
                    //Číslo v desítkové soustavě převedené z hexadecimální
                    unsigned long num;
                    //převod hexadecimálního čísla do desítkové soustavy
                    num = strtoul(StrRead(&unicode_number), NULL, 16);
                    
                    //sekvence \u{...} bude přeložena do tvaru \xyz 
                    if (num >= 100) {
                        snprintf(unicode_result, CHAR_CODE_LEN, "\\%ld", num);
                    } else {
                        snprintf(unicode_result, CHAR_CODE_LEN, "\\0%ld", num);
                    }
                    //A následné vložení do výsledného řetězce
                    strAdd(&decoded_string, unicode_result);
                    state = INIT_S;
                } 
                break;
        }

        //Přechod na další znak v řetězci
        i++;
    }

    StrDestroy(&unicode_number);

    return decoded_string;
}
/* Koniec súboru decode.c */
