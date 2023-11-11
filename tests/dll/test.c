#include "../../strR.h"
#include "../../dll.h"
#include <stdio.h>
#include <string.h>



#define TEST(cond)                                    \
    if (!(cond))                                      \
    {                                                 \
        printf("FAIL[ln %d]\t%s\n", __LINE__, #cond); \
        exit(EXIT_FAILURE); \
    }

#define TEST_LIST_STR_T(list, expected, get_oper) \
    do \
    { \
        str_T s; \
        StrInit(&s); \
        TEST((get_oper)((list), &s)); \
        TEST(strcmp(StrRead(&s), (expected)) == 0); \
        StrDestroy(&s); \
    } while (0)


DLLstr_T list_var;
DLLstr_T* list = &list_var;

void test() {
    DLLstr_Init(list);
    TEST(list->first == NULL);
    TEST(list->active == NULL);
    TEST(list->last == NULL);
    TEST(!DLLstr_IsActive(list));

    TEST(DLLstr_InsertLast(list, "second"));
    TEST_LIST_STR_T(list, "second", DLLstr_GetLast);

    TEST(DLLstr_InsertFirst(list, "first"));
    DLLstr_First(list);
    TEST_LIST_STR_T(list, "first", DLLstr_GetValue);

    TEST(DLLstr_InsertAfter(list, "new second"));
    
    TEST_LIST_STR_T(list, "first", DLLstr_GetValue);
    DLLstr_Next(list);
    TEST_LIST_STR_T(list, "new second", DLLstr_GetValue);

    DLLstr_Previous(list);
    TEST_LIST_STR_T(list, "first", DLLstr_GetValue);
    
    TEST(DLLstr_InsertBefore(list, "ZERO"));
    DLLstr_Previous(list);
    TEST_LIST_STR_T(list, "ZERO", DLLstr_GetValue);
    TEST_LIST_STR_T(list, "ZERO", DLLstr_GetFirst);

    DLLstr_DeleteAfter(list);
    TEST_LIST_STR_T(list, "ZERO", DLLstr_GetValue);
    DLLstr_Next(list);
    TEST_LIST_STR_T(list, "new second", DLLstr_GetValue);

    DLLstr_Dispose(list);
}

int main()
{
    test();

    printf("Everything OK\n");
    return 0;
}