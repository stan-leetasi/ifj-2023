/** Projekt IFJ2023
 * @file logErr.c
 * @brief Funkcie pre hlásenie chýb prekladu kódu či samotného prekladača
 * @author Michal Krulich
 * @date 12.11.2023
 */

#include <stdio.h>
#include <stdarg.h>
#include "logErr.h"
#include "parser.h"

 /**
  * @brief riadok posledného načítaného tokenu, pre prípady výpisu keď tkn == NULL
 */
static int last_tkn_ln = 1;
/**
 * @brief stĺpec posledného načítaného tokenu, pre prípady výpisu keď tkn == NULL
*/
static int last_tkn_col = 1;

void logErrCompiler(const char* msg) {
    fprintf(stderr, "[COMPILER ERROR] %s\n", msg);
}

void logErrCompilerMemAlloc() {
    logErrCompiler("memory allocation failed");
}

void logErrCodeAnalysis(const int err_code, const int ln, const int c, const char* format, ...) {
    if (err_code == LEX_ERR) fprintf(stderr, "Lexical Error");
    else if (err_code == SYN_ERR) fprintf(stderr, "Syntax Error");
    else fprintf(stderr, "Semantic Error");

    fprintf(stderr, " - ln %d, col %d: ", ln, c);

    va_list l;
    va_start(l, format);
    vfprintf(stderr, format, l);
    va_end(l);

    fprintf(stderr, "\n");
}

void logErrSyntax(const token_T* t, const char* expected) {
    if (t != NULL) logErrCodeAnalysis(SYN_ERR, t->ln, t->col, "expected %s, but got '%s'", expected, StrRead((str_T*)(&(t->atr))));
    else logErrCodeAnalysis(SYN_ERR, last_tkn_ln, last_tkn_col, "expected %s", expected);
}

void logErrSemantic(const token_T* t, const char* format, ...) {
    fprintf(stderr, "Semantic Error - ln %d, col %d: ",
        tkn != NULL ? t->ln : last_tkn_ln, tkn != NULL ? t->col: last_tkn_col);

    va_list l;
    va_start(l, format);
    vfprintf(stderr, format, l);
    va_end(l);

    fprintf(stderr, "\n");
}

void logErrSemanticFn(const char* fn, const char* format, ...) {
    fprintf(stderr, "Semantic Error - %s(): ", fn);

    va_list l;
    va_start(l, format);
    vfprintf(stderr, format, l);
    va_end(l);

    fprintf(stderr, "\n");
}

void logErrUpdateTokenInfo(const token_T *t) {
    last_tkn_ln = t->ln;
    last_tkn_col = t->col;
}


/* Koniec súboru logErr.c */
