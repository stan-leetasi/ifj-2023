/** Projekt IFJ2023
 * @file exp.c
 * @brief Precedenčná syntaktická a sémantická analýza výrazov s generovaním cieľového kódu
 * @author Stanislav Letaši
 * @date 14.11.2023
 */

#include "stdio.h"
#include "exp.h"
#include "strR.h"
#include "symtable.h"


/* Poznamky::

Syntaktická analýza: Check typov, zátvoriek, operátory, operandy, binárne unárne operátory
Sémantická analýza: Kompatibilita typov, Overenie či sú premenné a konštanty deklarované, scope analýza?, 


*/

/** Počas syntaktickej analýzy symbolizuje že ešte nebol spracovaný žiadny token*/
#define NO_PREV -1

/******************************************************************************************
* Štruktúry
*****************************************************************************************/

typedef struct parsed_token
{
    int type;       ///< typ tokenu
    char st_type;   ///< typ premennej/funkcia, používa hodnoty SYM_TYPE_XXX
    str_T id;       ///< názov identifikátoru, zároveň kľúč v tabuľke
    str_T codename; ///< identifikátor v cieľovom kóde
    bool init;      ///< true znamená, že je premenná inicializovaná alebo funkcia definovaná
} ptoken_T;

typedef struct stack
{
    int size; // Počet prvkov v zásobníku
    int capacity; // Kapacita zásobníku
    ptoken_T **array; // Dynamické pole pre uloženie tokenov

}stack_t;


/******************************************************************************************
 *Funkcie zásobníka
*****************************************************************************************/

/**
 * 
*/
bool Stack_Init( stack_t *stack ) {

    stack->array = malloc(sizeof(ptoken_T*)*16); // Alokácia pamäte pre pole
	if(stack->array == NULL){ // Alokácia pamäte zlyhala
        return false;
	}
	stack->size = 0; // 0 Iniciálnych položiek
    stack->capacity = 16; // Iniciálna kapacita pre 16 položiek
	
    return true; // Inicializácia prebehla úspešne
}

/**
 * @brief Vracia prvok z vrcholu zásobníka
*/
ptoken_T *stack_top(stack_t *stack){
    return(stack->array[stack->size]); // Vrátenie hodnoty z vrcholu zásobníka
}

/**
 * @brief Vloží parsed_token na zásobník
 * @returns 0 ak prebehlo vloženie tokenu úspešne, inak konkrétny chybový kód
*/
int stack_push_ptoken(stack_t *stack, ptoken_T *token){
    if(stack->capacity == stack->size){ // Ak je zásobník plný
        stack->capacity = stack->capacity*2; // Zdvojnásobenie kapacity
        if(realloc(stack->array, stack->capacity*sizeof(token_T)) == NULL){ // Realokácia pamäte zlyhala
            return COMPILER_ERROR;
        }
    }

    stack->array[stack->size] = token; // Vloženie tokenu na zásobník
    return 0; // Úspešné vloženie premennej na zásobník
}

/**
 * @brief Konvertuje token na parsed_token a zavolá stack_push_ptoken
 * @returns 0 ak prebehlo vloženie tokenu úspešne, inak konkrétny chybový kód
*/
int stack_push_token(stack_t *stack, token_T *token){
    
    TSData_T *symtabData = SymTabLookup(&symt, StrRead(&(token->atr))); // Získanie dát o premennej z tabuľky symbolov

    if(symtabData == NULL || symtabData->init == false){ // Premenná nebola deklarovaná alebo inicializovaná
        return SEM_ERR_UNDEF; // Chybový stav
    }

    ptoken_T *parsed_token = malloc(sizeof(ptoken_T)); // Nový parsed token
    if(parsed_token == NULL){ // Chyba pri alokácii
        return COMPILER_ERROR;
    }
    
    str_T id, codename;  // Reťazce pre identifikátor a id v cieľovom kóde
    StrInit(&id);
    StrInit(&codename);
    StrFillWith(&id, token->atr.data); // Vloženie názvu premennej do id

    parsed_token->id = id; // Id parsed tokenu
    parsed_token->type = token->type; // Typ tokenu
    parsed_token->st_type = symtabData->type; // Typ premennej v Tabuľke symbolov
    parsed_token->init = symtabData->init; // Informácia o inicializácii premennej
    parsed_token->codename = symtabData->codename; // Identifikátor v cieľovom kóde
    
    if(stack_push_ptoken(stack, parsed_token) == COMPILER_ERROR){ // Vloženie parsed tokenu na zásobník
        return COMPILER_ERROR; // Zlyhal realloc pamäte v stack_push_ptoken
    }

    return 0; // Úspešné vloženie tokenu na zásobník
}

/**
 * @brief Odstráni prvok z vrcholu zásobníka
*/
void stack_pop(stack_t *stack){
    stack->size = stack->size-1; // Zmenšenie počtu prvkov zásobníka
    stack->array[stack->size] = NULL; // Vymazanie ukazateľa na prvok zo zásobníku
}

/**
 * @brief Odstráni prvok z vrcholu zásobníka a zároveň uvoľní pamäť alokovanú pre prvok
*/
void stack_pop_destroy(stack_t *stack){
    ptoken_T *element; // Pomocná premenná na vyprázdnenie zásobníku
    element = stack_top(stack); // Odstraňujeme prvok z vrcholu
    if(element != NULL) // Prevencia double free
    {
        StrDestroy(&(element->id)); // Odstránenie strR id
        StrDestroy(&(element->codename)); // Odstránenie strR codename
        free(element); // Uvoľenie pamäte alokovanej pre prvok
    }
    stack_pop(stack);
}


/**
 * @brief Vyprázdni zásobník a uvoľní všetku alokovanú pamäť
*/
void stack_dispose(stack_t *stack){
    if(stack->array != NULL){ // Prevencia proti double free
        
        while(stack->size > 0) // Kým sa nevymažú všetky položky
        {
            stack_pop_destroy(stack); // Odstránenie prvku zo zásobníka a vymazanie
        }
        free(stack->array); // Uvoľnenie alokovanej pamäti zoznamu
    }

    stack->capacity = stack->size = 0; // Veľkosť a kapacita = 0
}

/******************************************************************************************
* Overenie typu tokenu
*****************************************************************************************/

/**
 * @brief Overuje, či token nie je jeden z typov ktoré sa nemôžu vo výraze vyskytovať.
 * @param type
 * @details Volaná pri každom tokene. Ak je false pri tokene na rovnakom riadku ako výraz => výraz nie je valídny. 
 * Inak indikuje koniec výrazu.
 * @returnss true ak môže token byť súčasťou výrazu, inak false
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
 * @returnss true ak je operand, inak false
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
 * @returnss true ak je operátor, inak false
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

/******************************************************************************************
 *Ostatné funkcie
*****************************************************************************************/


/**
 * @brief Funkcia zavolaná pred ukončením parseExpression
 * @details Uvoľní alokovanú pamäť pomocných premenných a zásobníku
**/
void endParse(stack_t *stack, stack_t *postfixExpr){
    stack_dispose(stack);
    stack_dispose(postfixExpr);
}


/**
 * @brief Porovná prioritu operátorov
 * @details 
 * @returns true ak má type väčšiu prioritu ako top, inak false
**/
bool priority_cmp(int type, int top){
    if(type == EXCL){ // !
        top = 0;
    }
    if(top == EXCL){ // !
        top = 0;
    }
    if(type == OP_MUL || type == OP_DIV){ // * /
        type = 1;
    }
    if(top == OP_MUL || top == OP_DIV){ // * /
        top = 1;
    }
    if(type == OP_PLUS || type == OP_MINUS){ // + -
        type = 2;
    }
    if(top == OP_PLUS || top == OP_MINUS){ // + -
        top = 2;
    }
    if(type > 29 && type < 36){ // == != < > <= >= 
        type = 3;
    }
    if(top > 29 && top < 36){ // == != < > <= >= 
        top = 3;
    }
    if(type == TEST_NIL){ // ??
        type = 4;
    }
    if(top == TEST_NIL){ // ??
        top = 4;
    }

    if(top > type){ // Operátor na vrchole zásobníka má menšiu prioritu ako operátor type
        return true;
    }
    return false; // Operátor na vrchole zásobníka má väčšiu prioritu ako operátor type
}


/**
 * @brief Konvertuje infixový výraz do postfixovej formy
 * @details Volaná počas syntaktickej analýzy výrazu pri každom tokene
 * @returns 0 ak prebehlo vloženie tokenu úspešne, inak konkrétny chybový kód
**/
int infix2postfix(stack_t *stack, stack_t *postfixExpr, token_T *infix_token){
    
    int status = 0; // Pomocná premenná pre uloženie návratového kódu funkcií

    if(infix_token->type == BRT_RND_L || infix_token->type == EXCL) // Ľavá zátvorka alebo "!"
	{
		return stack_push_token(stack, infix_token); // Vkladáme na zásobník
	}
	if(infix_token->type == BRT_RND_R) // Pravá zátvorka
	{
		while(true) // Pokým nenájdeme ľavú zátvorku
	    {
		    if(stack->size == 0){ // Ukončenie funkcie pri prázdnom zásobníku
		    	return 0;
		    }

		    if(stack_top(stack)->type == BRT_RND_L) // Ľavá zátvorka bola nájdená
		    {
		    	stack_pop_destroy(stack); // Odstránenie zátvorky zo zásobníku a z pamäte
		    	return 0; // Ukončenie funkcie
		    }
		    else
		    {
		    	status = stack_push_ptoken(postfixExpr, stack_top(stack)); // Pridanie znaku zo zásobníku do výrazu
                if(status != 0){ // Push neprebehol úspešne
                    return status; // Vrátenie chybového kódu
                }
		    	stack_pop(stack); // Odstránenie znaku zo zásobníku
		    }
	    }
		return 0; // Úspešné prevedenie operácie
	}
	if(!valid_token_type(infix_token->type))
	{	
		while(stack->size > 0) // Pokiaľ sa nevyprázdni celý zásobník
		{
			status = stack_push_ptoken(postfixExpr, stack_top(stack)); // Vloženie prvku z vrcholu zásobníka do postfixExpression
            if(status != 0){ // Push neprebehol úspešne
                    return status; // Vrátenie chybového kódu
            }
            stack_pop(stack); // Odstránenie prvku zo zásobníku
		}

		return 0; // Úspešné prevedenie operácie
	}
	if(is_binary_operator(infix_token->type))
	{
		while (true) // Opakovanie odstraňovania operátorov zo zásobníka až dokým nebude možné vložiť znak c na zásobník
		{
			if(stack->size == 0){// Zásobník je prázdny
				return stack_push_token(stack, infix_token); // Vloženie znaku na zásobník, koniec funkcie
			}
			else // Zásobník nie je prázdny
			{
                // Na vrchole zásobníka je ľavá zátvorka alebo operátor s nižšou prioritou ako infix_token
				if(stack_top(stack)->type == BRT_RND_L || priority_cmp(infix_token->type, stack_top(stack)->type)){ 
					return stack_push_token(stack, infix_token); // Vloženie znaku na zásobník, koniec funkcie
				}
				else
                {
					status = stack_push_ptoken(postfixExpr, stack_top(stack)); // Vloženie operátoru s rovnakou/vyššou prioritou na koniec postfix výrazu
                    if(status != 0){ // Push neprebehol úspešne
                        return status; // Vrátenie chybového kódu
                    }
					stack_pop(stack); // Odstránenie operátoru zo zásobníka
				}
			}
	    }
	}

	else // Žiadna predošlá podmienka nebola splnená => znak musí byť operand
	{
		return stack_push_token(stack, infix_token); // Vloženie operandu na zásobník, koniec funkcie
	}


    return 0;
}

/******************************************************************************************
 *Hlavná funkcia
*****************************************************************************************/

int parseExpression(char* result_type) {

    stack_t stack; // Zásobník pre konverziu výrazu na postfixovú formu
    stack_t postfixExpr; // Zásobník pre uloženie postfixového výrazu
    Stack_Init(&stack); // Inicializácia zásobníka
    Stack_Init(&postfixExpr); // Inicializácia zásobníka

    int prevTokenType = NO_PREV; // Pomocná premenná pre uloženie typu tokenu pred momentálne spracovaným
    int bracketCount = 0; // Premenná na overenie korektnosti zátvoriek "()" vo výraze
    int status = 0; // Premenná na overenie priebehu volania funkcie
    
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
                
                if(infix2postfix(&stack, &postfixExpr, tkn) == COMPILER_ERROR){ // Signalizuje ukončenie postfix výrazu
                    return COMPILER_ERROR; // Nastala chyba pri malloc/realloc
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
                        if(infix2postfix(&stack, &postfixExpr, tkn) == COMPILER_ERROR){ // Signalizuje ukončenie postfix výrazu
                            return COMPILER_ERROR; // Nastala chyba pri malloc/realloc
                        }
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
            if(!is_operand(prevTokenType) && prevTokenType != BRT_RND_R) // Predošlý token nie je operand alebo "("
            {
                prevTokenType = NO_PREV;
                break; // Výraz nie je valídny
            }

        }

        
        status = infix2postfix(&stack, &postfixExpr, tkn); // Pridanie tokenu do postfix výrazu
        if(status != 0) // Pridanie tokenu do postfix výrazu nebolo úspešné
        {
            endParse(&stack, &postfixExpr); // Upratenie pred ukončením
            return status; // Vrátenie chybového kódu            
        }

        prevTokenType = tkn->type; // Uloženie typu predošlého tokenu
        status = nextToken(); // Požiadanie o ďalší token z výrazu
        if(status != 0){ // nextToken vrátil chybu
            return status; // Vrátenie chybovej hodnoty
        }

    }

    if(prevTokenType == NO_PREV) // Symbolizuje chybnú syntax
    {
        endParse(&stack, &postfixExpr); // Upratanie pred skončením funkcie
        return SYN_ERR; // Vrátenie chybového stavu
    }



    
    
    
    return COMPILATION_OK;
}


/* Koniec súboru exp.c */
