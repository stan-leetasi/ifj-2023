/** Projekt IFJ2023
 * @file exp.c
 * @brief Precedenčná syntaktická a sémantická analýza výrazov s generovaním cieľového kódu
 * @author Stanislav Letaši
 * @date 14.11.2023
 */

#include "exp.h"
#include "strR.h"
#include "stdio.h"
/* Poznamky::

Syntaktická analýza: Check typov, zátvoriek, operátory, operandy, binárne unárne operátory
Sémantická analýza: Kompatibilita typov, Overenie či sú premenné a konštanty deklarované, scope analýza?, 


*/

/** Počas syntaktickej analýzy symbolizuje že ešte nebol spracovaný žiadny token*/
#define NO_PREV -1

typedef struct stack
{
    int size; // Počet prvkov v zásobníku
    int capacity; // Kapacita zásobníku
    token_T **array; // Dynamické pole pre uloženie tokenov

}stack_t;


bool Stack_Init( stack_t *stack ) {

    stack->array = malloc(sizeof(token_T*)*16); // Alokácia pamäte pre pole
	if(stack->array == NULL){ // Alokácia pamäte zlyhala
        return false;
	}
	stack->size = 0; // 0 Iniciálnych položiek
    stack->capacity = 16; // Iniciálna kapacita pre 16 položiek
	
    return true; // Inicializácia prebehla úspešne
}

bool stack_push(stack_t *stack, token_T *token){
    if(stack->capacity == stack->size){ // Ak je zásobník plný
        stack->capacity = stack->capacity*2; // Zdvojnásobenie kapacity
        if(realloc(stack->array, stack->capacity*sizeof(token_T)) == NULL){ // Realokácia pamäte zlyhala
            return false;
        }
    }

    stack->array[stack->size] = token; // Vloženie tokenu na zásobník
    return true; // Úspešné vloženie premennej na zásobník
}

void stack_pop(stack_t *stack){
    stack->size = stack->size-1; // Zmenšenie počtu prvkov zásobníka
    stack->array[stack->size-1] = NULL; // Vymazanie ukazateľa na prvok zo zásobníku
}

token_T *stack_top(stack_t *stack){
    return(stack->array[stack->size]); // Vrátenie hodnoty z vrcholu zásobníka
}

void stack_dispose(stack_t *stack){
    if(stack->array != NULL){ // Prevencia proti double free
        free(stack->array); // Uvoľnenie alokovanej pamäti
    }
    stack->capacity = stack->size = 0; // Veľkosť a kapacita = 0
}

/**
* @brief Overuje, či token nie je jeden z typov ktoré sa nemôžu vo výraze vyskytovať.
* @param type
* @details Volaná pri každom tokene. Ak je false pri tokene na rovnakom riadku ako výraz => výraz nie je valídny. Inak indikuje koniec výrazu.
* @returns true ak môže token byť súčasťou výrazu, inak false
* 
**/
bool valid_token_type(int type){
    if (type == INVALID || type == INT_TYPE || type == DOUBLE_TYPE || type == ELSE ||
        type == STRING_TYPE || type == INT_NIL_YPE || type == DOUBLE_NIL_TYPE ||
        type == STRING_NIL_TYPE || type == VAR || type == LET || type == IF ||
        type == WHILE || type == FUNC || type == RETURN || type == BRT_CUR_R ||
        type == UNDERSCORE || type == ARROW || type == ASSIGN || type == BRT_CUR_L ||
        type == QUEST_MARK || type == COMMA || type == COLON || type == EOF_TKN) {
        return false;
    }

    else{
        return true; // Token môže byť časťou výrazu
    }
}

/**
* @brief Overuje, či je token operand.
* @param type
* @returns true ak je operand, inak false
**/
bool is_operand(int type){
    if(type == ID || type == INT_CONST || type == DOUBLE_CONST ||
    type == STRING_CONST || type == NIL){
        return true;
    }
    else{
        return false;
    }
}

/**
* @brief Overuje, či je token binárny operátor.
* @param type
* @returns true ak je operátor, inak false
* @note Operátor "-" je v tejto implementácii iba binárny
**/
bool is_binary_operator(int type){
    if (type == OP_PLUS || type == OP_MUL || type == OP_DIV || type == OP_MINUS ||
        type == ASSIGN || type == EQ || type == NEQ || type == GTEQ ||
        type == LT || type == LTEQ || type == TEST_NIL || type == GT) {
        return true;
    }
    else{
        return false;
    }
}


/**
* @brief Funkcia zavolaná pred ukončením parseExpression
* @details Uvoľní alokovanú pamäť pomocných premenných a zásobníku
**/
void endParse(stack_t *stack, stack_t *postfixExpr){
    stack_dispose(stack);
    stack_dispose(postfixExpr);
}

void infix2postfix(stack_t *stack, token_T *infix_token){
    
}



int parseExpression(char* result_type) {

    stack_t stack; // Zásobník pre konverziu výrazu na postfixovú formu
    stack_t postfixExpr; // Zásobník pre uloženie postfixového výrazu
    Stack_Init(&stack); // Inicializácia zásobníka
    Stack_Init(&postfixExpr); // Inicializácia zásobníka


    int prevTokenType = NO_PREV; // Pomocná premenná pre uloženie typu tokenu pred momentálne spracovaným
    int bracketCount = 0; // Premenná na overenie korektnosti zátvoriek "()" vo výraze
    
    // Syntaktická analýza
    while(true) // Pokým sa nespracuje celý výraz
    {
        if(!valid_token_type(tkn->type)) // Ak token nemôže patriť do výrazu 
        {
            if(tkn->type == 0) // Token je typu INVALID
            {
                endParse(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                return LEX_ERR; // Lexikálna chyba
            }
            if(prevTokenType == NO_PREV){ // Token je prvý vo výraze
                break; // Výraz nie je valídny
            }
            else // Predpokladáme že token už nie je súčasťou výrazu => znamená to ukončenie výrazu
            {
                if(is_binary_operator(prevTokenType)) // Predošlý token je binárny operátor
                {
                    prevTokenType = NO_PREV;
                    break; // Finálny výraz nie je valídny
                }
                if(bracketCount != 0) // Vo výraze nie sú uzatvorené všetky zátvorky
                {
                    prevTokenType = NO_PREV;
                    break; // Finálny výraz nie je valídny
                }
                
                saveToken(); // Vloženie tokenu späť do input streamu
                break; // Koniec syntaktickej analýzy výrazu
            }
        }

        if(is_binary_operator(tkn->type)) // Binárny operátor
        {
            if(prevTokenType == NO_PREV || is_binary_operator(prevTokenType) || prevTokenType == BRT_RND_L) // Token je prvý vo výraze, je za binárnym operátorom alebo ľavou zátvorkou
            {
                prevTokenType = NO_PREV;
                break; // Výraz nie je valídny
            }
        }

        if(is_operand(tkn->type)) // Operand
        {
            if(prevTokenType != NO_PREV) // Operand nie je prvý token vo výraze
            {
                if(is_operand(prevTokenType) || prevTokenType == EXCL || (prevTokenType == BRT_RND_R)) // Predošlý token je operand, "!" alebo pravá zátvorka
                {
                    if(bracketCount != 0) // Ak nie sú uzavreté všetky zátvorky
                    {
                        prevTokenType = NO_PREV;
                        break; // Výraz nie je valídny
                    }
                    else
                    {
                        saveToken(); // Vloženie tokenu späť do input streamu
                        break; // Koniec syntaktickej analýzy výrazu
                    }
                }
            }
        }
        if(tkn->type == BRT_RND_L) // Ľavá zátvorka
        {
            if(prevTokenType != NO_PREV && !is_binary_operator(prevTokenType) && prevTokenType != BRT_RND_L) // Token nie je prvý vo výraze a predošlý token nie je binárny operátor
            {
                prevTokenType = NO_PREV;
                break; // Výraz nie je valídny
            }
            else{
            bracketCount ++; // Zvýšenie počtu otvorených ľavých zátvoriek
            }
        }
        if(tkn->type == BRT_RND_R) // Pravá zátvorka
        {
            if(is_binary_operator(prevTokenType) || bracketCount == 0) // Predošlý token je binárny operátor alebo vo výraze nie je otvorená zátvorka
            {
                prevTokenType = NO_PREV;
                break; // Výraz nie je valídny
            }

            bracketCount--; // Uzatvorenie páru zátvoriek

        }
        if(tkn->type == EXCL) // Výkričník
        {
            if(!is_operand(prevTokenType)) // Predošlý token nie je operand
            {
                prevTokenType = NO_PREV;
                break; // Výraz nie je valídny
            }

        }

        
        infix2postfix(&stack, tkn); // Pridanie tokenu do postfix výrazu
        prevTokenType = tkn->type; // Uloženie typu predošlého tokenu
        tkn = getToken(); // Požiadanie o ďalší token z výrazu

    }

    if(prevTokenType == NO_PREV) // Symbolizuje chybnú syntax
    {
        endParse(&stack, &postfixExpr); // Upratanie pred skončením funkcie
        return SYN_ERR; // Vrátenie chybového stavu
    }



    
    
    
    return COMPILATION_OK;
}


/* Koniec súboru exp.c */
