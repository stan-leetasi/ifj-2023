/** Projekt IFJ2023
 * @file strR.c
 * @brief Resizable string - reťazec s automatickou realokáciu veľkosti
 * @author Boris Hatala (xhatal02)
 * @date 18.10.2023
 */

#include "strR.h"

void StrInit(str_T *s)
{
    s -> data = (char *)malloc(STR_INIT_SIZE * sizeof(char));

    if(s -> data == NULL) {
        fprintf(stderr, "StrInit() memory allocation error.\n");
        exit(99);
    }

    // inicializacia prveho znaku na '\0'
    s -> data[0] = '\0';
    s -> size = STR_INIT_SIZE;

}

void StrDestroy(str_T *s)
{
    free(s -> data);
    s -> data = NULL;
    s -> size = 0;
}

char *StrRead(str_T *s)
{
    return s -> data;
}

void StrAppend(str_T *s, char c)
{
    // realokovat ak je potrebne
    if (strlen(s->data) + 1 == s->size) {
        s->size *= 2;
        s->data = (char *)realloc(s->data, s->size);

        if (s->data == NULL) {
            fprintf(stderr, "StrAppend() memory allocation error.\n");
            exit(99);
        }
    }

    // pridat znak na koniec a posunut koniec retezca
    size_t len = strlen(s->data);
    s->data[len] = c;
    s->data[len + 1] = '\0';
}

void StrFillWith(str_T *dest, char *src)
{
    size_t len = strlen(src);

    // realokovat ak je potrebne na len + 1
    if (len >= dest->size) {
        dest->size = len + 1;
        dest->data = (char *)realloc(dest->data, dest->size);

        if (dest->data == NULL) {
            fprintf(stderr, "StrFillWith() memory allocation error..\n");
            exit(99);
        }
    }

    strcpy(dest->data, src);
}

void StrCat(str_T *dest, str_T *src)
{
    size_t len_dest = strlen(dest->data);
    size_t len_src = strlen(src->data);
    
    // realokovat ak je potrebne na dlzku dest + dlzku src + 1
    if (len_dest + len_src + 1 >= dest->size) {
        dest->size = len_dest + len_src + 1;
        dest->data = (char *)realloc(dest->data, dest->size);
        
        if (dest->data == NULL) {
            fprintf(stderr, "StrCat() memory allocation error.\n");
            exit(99);
        }
    }
    
    strcat(dest->data, src->data);
}

void StrCatString(str_T *dest, char *src)
{
    size_t len_dest = strlen(dest->data);
    size_t len_src = strlen(src);
    
    // realokovat ak je potrebne na dlzku dest + dlzku src + 1
    if (len_dest + len_src + 1 >= dest->size) {
        dest->size = len_dest + len_src + 1;
        dest->data = (char *)realloc(dest->data, dest->size);
        
        if (dest->data == NULL) {
            fprintf(stderr, "StrCatString() memory allocation error.\n");
            exit(99);
        }
    }

    strcat(dest->data, src);
}
/* Koniec súboru strR.c */
