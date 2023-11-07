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

void logErrCodeAnalysis(const int err_code, const size_t ln, const size_t c, const char *format, ...) {
    if(err_code == LEX_ERR) fprintf(stderr, "Lexical Error");
    else if (err_code == SYN_ERR) fprintf(stderr, "Syntax Error");
    else fprintf(stderr, "Semantic Error");
    
    fprintf(" - ln %d, col %d: ", ln, c);
    
    va_list l;
    va_start(l, format);
    vfprintf(stderr, format, l);
    va_end(l);

    fprintf(stderr, "\n");
}

/* Koniec súboru logErr.c */
