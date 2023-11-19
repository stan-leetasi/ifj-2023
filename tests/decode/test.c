#include "../../strR.h"
#include "../../decode.h"
#include <stdio.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

char line[MAX_LINE_LENGTH];

int main()
{
    while (fgets(line, MAX_LINE_LENGTH, stdin) != NULL)
    {
        line[strlen(line) - 1] = '\0'; // odstrániť znak nového riadku od fgets
        
        str_T output = strEncode(line);
        
        printf("%s\n", StrRead(&output));
        StrDestroy(&output);
    }
    return 0;
}