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

enum codes {
    SPACE, 
    BACKSLASH, 
    NEW_LINE, 
    HASHTAG,
    QUOTE,
    CARRIAGE_RETURN,
    TAB,
};

char *string_codes[] = {
    "\\032",    //mezera
    "\\092",    //zpětné lomítko 
    "\\010",    //nový řádek
    "\\035",    //mřížka #
    "\\\"",     //uvozovka
    "\\r",      //přesun kurzoru na začátek
    "\\t"       //odsazení
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
    //zde bude uložený nový řetězec
    str_T decoded_string;

    //Inicializace datové struktury str_T
    StrInit(&decoded_string);
    //Iterátor - touto proměnnou se bude iterovat znak po znaku řetězecv "string"
    unsigned i = 0;
    //Zde bude konkrétní znak ze "string"
    int c;
    //Na začátek řetězce přidá prefix string@
    StrFillWith(&decoded_string, "string@");

    //Procházení řetězce "string" znak po znaku
    while((c = string[i]) != '\0') {
        
        switch (c)
        {
        case ' ':  
            //mezera
            strAdd(&decoded_string, string_codes[SPACE]);

            break;
        case '\\':
            //zpětné lomítko
            strAdd(&decoded_string, string_codes[BACKSLASH]);

            break;
        case '\n':
            //Nový řádek
            strAdd(&decoded_string, string_codes[NEW_LINE]);

            break;
        case '#':
            //Hashtag
            strAdd(&decoded_string, string_codes[HASHTAG]);

            break;
        case '\"':
            //Uvozovka
            strAdd(&decoded_string, string_codes[QUOTE]);
            break;
        case '\r':
            //Přesun kurzoru na začátek
            strAdd(&decoded_string, string_codes[CARRIAGE_RETURN]);
            break;
        case '\t':
            //Odsazení
            strAdd(&decoded_string, string_codes[TAB]);
            break;
        default:

            StrAppend(&decoded_string, c);
            break;
        }

        //Přechod na další znak v řetězci
        i++;
    }

    return decoded_string;
}