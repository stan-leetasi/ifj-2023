#include "../../strR.h"
#include <stdio.h>
#include <string.h>

int failures = 0;

#define TEST(cond)                                    \
    if (!(cond))                                      \
    {                                                 \
        printf("FAIL[ln %d]\t%s\n", __LINE__, #cond); \
        failures++;                                   \
    }

int main()
{
    str_T s;
    StrInit(&s);
    TEST(s.size == STR_INIT_SIZE);
    TEST(s.data[0] == '\0');
    TEST(StrRead(&s)[0] == '\0');

    StrFillWith(&s, "C retazec");
    TEST(strcmp(StrRead(&s), "C retazec") == 0);
    
    str_T alphabet;
    StrInit(&s);
    for(char c='a'; c<='z'; c++)
    {
        StrAppend(&alphabet, c);
    }
    TEST(strcmp(StrRead(&alphabet), "abcdefghijklmnopqrstuvwxyz") == 0);

    StrCat(&alphabet, &s);
    TEST(strcmp(StrRead(&alphabet), "abcdefghijklmnopqrstuvwxyzC retazec") == 0);

    if(failures != 0)
    {
        printf("Total tests failed: %d\n", failures);
    }
    else{
        printf("Everything OK\n");
    }
    return 0;
}