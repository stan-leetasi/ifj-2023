/** Projekt IFJ2023
 * @file logErr.c
 * @brief Funkcie pre hlásenie chýb prekladu kódu či samotného prekladača
 * @author Michal Krulich
 * @date 02.11.2023
 */

#include <stdio.h>
#include <stdarg.h>
#include "logErr.h"
#include "parser.h"

void logErrCompiler(const char *msg) {
    fprintf(stderr, "[COMPILER ERROR] %s\n", msg);
}

void logErrCompilerMemAlloc() {
    logErrCompiler("memory allocation failed");
}

void logErrCodeAnalysis(const int err_code, const int ln, const int c, const char *format, ...) {
    if(err_code == LEX_ERR) fprintf(stderr, "Lexical Error");
    else if (err_code == SYN_ERR) fprintf(stderr, "Syntax Error");
    else fprintf(stderr, "Semantic Error");
    
    fprintf(stderr, " - ln %d, col %d: ", ln, c);
    
    va_list l;
    va_start(l, format);
    vfprintf(stderr, format, l);
    va_end(l);

    fprintf(stderr, "\n");
}

void logErrSyntax(const token_T *t, const char *expected) {
    logErrCodeAnalysis(SYN_ERR, t->ln, t->col, "expected %s, but got '%s'", expected, StrRead((str_T*)(&(t->atr))));
}

/* Koniec súboru logErr.c */
