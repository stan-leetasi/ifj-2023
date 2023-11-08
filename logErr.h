/** Projekt IFJ2023
 * @file logErr.h
 * @brief Funkcie pre hlásenie chýb prekladu kódu či samotného prekladača
 * @author Michal Krulich
 * @date 02.11.2023
 */

#ifndef _LOGERR_H_
#define _LOGERR_H_ 1

/**
 * @brief Vypíše do stderr chybu prekladača s priloženou správou
*/
void logErrCompiler(const char *msg);

/**
 * @brief Vypíše do stderr chybu alokácie pamäte
*/
void logErrCompilerMemAlloc();

/**
 * @brief Vypíše do stderr chybu prekladu kódu, funguje podobne ako printf
*/
void logErrCodeAnalysis(const int err_code, const int ln, const int c, const char *format, ...);

/**
 * @brief Vypíše do stderr informáciu o syntaktickej chybe
*/
void logErrSyntax(const token_T *t, const char *expected);

#endif

/* Koniec súboru logErr.h */
