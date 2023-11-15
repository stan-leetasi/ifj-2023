#include "../../generator.h"
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

    int test = genUniqVar("GF", "x", &s);
    TEST(strcmp(StrRead(&s), "GF@x$1") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genUniqVar("GF", "x", &s);
    TEST(strcmp(StrRead(&s), "GF@x$2") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genUniqLabel("pow", "while", &s);
    TEST(strcmp(StrRead(&s), "pow&while1") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genUniqLabel("fib", "while", &s);
    TEST(strcmp(StrRead(&s), "fib&while2") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genConstVal(8, "123", &s);
    TEST(strcmp(StrRead(&s), "int@123") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genConstVal(9, "3.14", &s);
    TEST(strcmp(StrRead(&s), "float@0x1.91eb86p+1") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genConstVal(18, "", &s);
    TEST(strcmp(StrRead(&s), "nil@nil") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genConstVal(10, "a b", &s);
    TEST(strcmp(StrRead(&s), "string@a\\032b") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genConstVal(-50, "true", &s);
    TEST(strcmp(StrRead(&s), "bool@true") == 0);

    StrDestroy(&s);
    StrInit(&s);

    test = genConstVal(2, "true", &s);
    TEST(test == COMPILER_ERROR);
    StrDestroy(&s);

    test = genCode("ADD", "GF@x", "GF@y", "LF@z");
    if(parser_inside_fn_def) {
        TEST(strcmp(code_fn.last->string, "ADD GF@x GF@y LF@z") == 0);
    }
    else {
        TEST(strcmp(code_main.last->string, "ADD GF@x GF@y LF@z") == 0);
    }


    if(failures != 0)
    {
        printf("Total tests failed: %d\n", failures);
    }
    else{
        printf("Everything OK\n");
    }
}