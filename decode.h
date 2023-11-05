/**
 * @file decode.h
 * @author František Holáň (xholan13@stud.fit.vut.cz)
 * @brief 
 * @version 0.1
 * @date 2023-10-28
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "strR.h"
/**
 * @brief Převede řetězec napsaný ve zdrojovém jazyce na řetězec pro IFJcode23
 * 
 * @param string Vstupní řetězec ve zdrojovém jazyce
 * @return Funkce vrací přímo převedený řetězec do IFJcode23 (strukturu str_T)
 */
str_T strEncode(char * string);