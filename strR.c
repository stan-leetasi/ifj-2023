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
        exit(1);
    }

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
    if (strlen(s->data) + 1 == s->size) {
        s->size *= 2;
        s->data = (char *)realloc(s->data, s->size);

        if (s->data == NULL) {
            fprintf(stderr, "StrAppend() memory allocation error.\n");
            exit(1);
        }
    }

    size_t len = strlen(s->data);
    s->data[len] = c;
    s->data[len + 1] = '\0';
}

void StrFillWith(str_T *dest, char *src)
{
    size_t len = strlen(src);

    if (len >= dest->size) {
        dest->size = len + 1;
        dest->data = (char *)realloc(dest->data, dest->size);

        if (dest->data == NULL) {
            fprintf(stderr, "StrFillWith() memory allocation error..\n");
            exit(1);
        }
    }

    strcpy(dest->data, src);
}

void StrCat(str_T *dest, str_T *src)
{
    size_t len_dest = strlen(dest->data);
    size_t len_src = strlen(src->data);
    
    if (len_dest + len_src + 1 >= dest->size) {
        dest->size = len_dest + len_src + 1;
        dest->data = (char *)realloc(dest->data, dest->size);
        
        if (dest->data == NULL) {
            fprintf(stderr, "StrCat() memory allocation error.\n");
            exit(1);
        }
    }
    
    strcat(dest->data, src->data);
}

void StrCatString(str_T *dest, char *src)
{
    size_t len_dest = strlen(dest->data);
    size_t len_src = strlen(src);
    
    
    if (len_dest + len_src + 1 >= dest->size) {
        dest->size = len_dest + len_src + 1;
        dest->data = (char *)realloc(dest->data, dest->size);
        
        if (dest->data == NULL) {
            fprintf(stderr, "StrCatString() memory allocation error.\n");
            exit(1);
        }
    }

    strcat(dest->data, src);
}
/* Koniec súboru strR.c */
