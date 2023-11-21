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
#include "generator.h"



/* Poznamky::

Syntaktická analýza: Check typov, zátvoriek, operátory, operandy, binárne unárne operátory
Sémantická analýza: Kompatibilita typov, scope analýza?, 


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
bool stack_init( stack_t *stack ) {

    stack->array = calloc(16, sizeof(ptoken_T*)); // Alokácia pamäte pre pole
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
    if(stack->size > 0){
    return(stack->array[stack->size - 1]); // Vrátenie hodnoty z vrcholu zásobníka
    }
    return NULL;
}

/**
 * @brief Vloží parsed_token na zásobník
 * @returns 0 ak prebehlo vloženie tokenu úspešne, inak konkrétny chybový kód
*/
int stack_push_ptoken(stack_t *stack, ptoken_T *token){
    if(stack->capacity == stack->size){ // Ak je zásobník plný
        stack->capacity = stack->capacity*2; // Zdvojnásobenie kapacity
        stack->array = realloc(stack->array, stack->capacity*sizeof(token_T));
        if(stack->array == NULL){ // Realokácia pamäte zlyhala
            return COMPILER_ERROR;
        }
    }

    stack->array[stack->size] = token; // Vloženie tokenu na zásobník
    stack->size = stack->size + 1; // Zväčšenie počtu prvkov v zásobníku
    return 0; // Úspešné vloženie premennej na zásobník
}

/**
 * @brief Konvertuje token na parsed_token a zavolá stack_push_ptoken
 * @returns 0 ak prebehlo vloženie tokenu úspešne, inak konkrétny chybový kód
*/
int stack_push_token(stack_t *stack, token_T *token){
    
    TSData_T *symtabData; // Premenná pre uloženie dát z tabuľky symbolov

    if(token->type == ID) // Ak je token identifikátor, musíme ho vyhľadať v tabuľke symbolov
    {
        symtabData = SymTabLookup(&symt, StrRead(&(token->atr))); // Získanie dát o premennej z tabuľky symbolov

        if(symtabData == NULL){ // Premenná nebola deklarovaná alebo inicializovaná
            return SEM_ERR_UNDEF; // Chybový stav
        }
        if(symtabData->init == false){ // Premenná nebola inicializovaná
            return SEM_ERR_UNDEF; // Chybový stav
        }
    }

    ptoken_T *parsed_token = malloc(sizeof(ptoken_T)); // Nový parsed token
    if(parsed_token == NULL){ // Chyba pri alokácii
        return COMPILER_ERROR;
    }
    
    str_T id, codename;  // Reťazce pre identifikátor, id a konštantnú hodnotu v cieľovom kóde
    StrInit(&id); // Inicializácia id
    StrInit(&codename); // Inicializácia codename
    StrFillWith(&id, token->atr.data); // Vloženie názvu premennej do id

    parsed_token->id = id; // Id parsed tokenu
    parsed_token->type = token->type; // Typ tokenu
    parsed_token->init = true; // Token je inicializovaný

    if(token->type == ID) // Operand je premenná
    {
        parsed_token->st_type = symtabData->type; // Typ premennej v Tabuľke symbolov
        StrFillWith(&codename, StrRead(&(symtabData->codename))); // Vloženie názvu premennej do id
        parsed_token->codename = codename; // Identifikátor v cieľovom kóde
    }
    //printf("1. parsed_token %s, type = %c\n", StrRead(&parsed_token->id), parsed_token->st_type);
    if(token->type == INT_CONST || token->type == DOUBLE_CONST || token->type == STRING_CONST || token->type == NIL)// Operand je konštanta
    {
        genConstVal(token->type, StrRead(&(tkn->atr)), &codename);

        parsed_token->codename = codename; // Identifikátor v cieľovom kóde (v prípade konštanty prázdny reťazec?)
        switch (parsed_token->type){
        case INT_CONST:
             parsed_token->st_type = 'i';
             break;

        case DOUBLE_CONST:
             parsed_token->st_type = 'd';
             break;

        case STRING_CONST:
             parsed_token->st_type = 's';
             break;

        case NIL:
             parsed_token->st_type = 'N';
             break;
        }
    }

    if(token->type != INT_CONST && token->type != DOUBLE_CONST && 
    token->type != STRING_CONST && token->type != NIL && token->type != ID) // Token je operátor
    {
        parsed_token->st_type = '0'; // Typ premennej (operátor nemá typ premennej)
        parsed_token->codename = codename; // Prázdný inicializovaný StrR
    }
    //printf("2. parsed_token %s, type = %c\n", StrRead(&parsed_token->id), parsed_token->st_type);
    return stack_push_ptoken(stack, parsed_token); // Vloženie parsed tokenu na zásobník a vrátenie return value
}

/**
 * @brief Odstráni prvok z vrcholu zásobníka
*/
void stack_pop(stack_t *stack){
    if(stack->size > 0)
    {
        stack->size = stack->size-1; // Zmenšenie počtu prvkov zásobníka
        stack->array[stack->size] = NULL; // Vymazanie ukazateľa na prvok zo zásobníku
    }
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
 * @brief Vyprázdni zásobník
*/
void stack_clear(stack_t *stack){
    if(stack->array != NULL){ // Prevencia proti double free
        
        while(stack->size > 0) // Kým sa nevymažú všetky položky
        {
            stack_pop(stack); // Odstránenie prvku zo zásobníka a vymazanie
        }
    }
    stack->size = 0; // Veľkosť = 0
}

/**
 * @brief Vyprázdni zásobník a uvoľní alokovanú pamäť každého prvku
*/

void stack_clear_free(stack_t *stack){
    if(stack->array != NULL){ // Prevencia proti double free
        
        while(stack->size > 0) // Kým sa nevymažú všetky položky
        {
            free(stack_top(stack));
            stack_pop(stack); // Odstránenie prvku zo zásobníka a vymazanie
        }
    }
    free(stack->array);
    stack->size = 0; // Veľkosť = 0
}

/**
 * @brief Vyprázdni zásobník a uvoľní všetku alokovanú pamäť
*/
void stack_dispose(stack_t *stack){
    if(stack->array != NULL){ // Prevencia proti double free
        
        while(stack->size > 0) // Kým sa nevymažú všetky položky
        {
            //printf("Dispose current size: %i\n",stack->size);
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
 * @brief Overuje, či je token aritmetický operátor.
 * @returns true ak je aritmetický operátor, inak false
**/
bool is_arithmetic_operator(int operator){
    if(operator == OP_PLUS || operator == OP_MUL || 
    operator == OP_DIV || operator == OP_MINUS){
        return true;
    }
    else{
        return false;
    }
}
/**
 * @brief Overuje, či je token logický operátor.
 * @returns true ak je aritmetický operátor, inak false
**/
bool is_logical_operator(int operator){
    if(operator == EQ || operator == NEQ || operator == GTEQ ||
        operator == LT || operator == LTEQ || operator == GT){
            return true;
        }
    else{
        return false;
    }
}

/**
 * @brief Zistí, či sú dátové typy operandov kompatibilné pre logické operácie
 * @returns true ak sú kompatibilné, inak false
**/
bool are_compatible_l(ptoken_T *op1, ptoken_T *op2){

    if(op1->st_type == 'b'){ // Bool
        return (op2->st_type == 'b');
    }
    if(op1->st_type == 'i'){ // Integer
        return (op2->type == INT_CONST || op2->st_type == 'i');
    }
    if(op1->type == INT_CONST){ // Integer konštanta - môže byť pretypovaná na double
        return (op2->type == INT_CONST || op2->st_type == 'i' || op2->st_type == 'd' || op2->st_type == DOUBLE_CONST);
    }
    if(op1->type == DOUBLE_CONST || op1->st_type == 'd'){ // Double
        return (op2->type == DOUBLE_CONST || op2->st_type == 'd' || op2->st_type == INT_CONST); // Druhý op môže byť pretypovaný na double
    }
    if(op1->type == STRING_CONST || op1->st_type == 's'){ // String
        return (op2->type == STRING_CONST || op2->st_type == 's');
    }
    if(op1->type == NIL || op1->st_type == 'I'){ // Integer NIL
        return (op2->type == NIL || op2->st_type == 'I');
    }
    if(op1->type == NIL || op1->st_type == 'D'){ // Double NIL
        return (op2->type == NIL || op2->st_type == 'D');
    }
    if(op1->type == NIL || op1->st_type == 'S'){ // String NIL
        return (op2->type == NIL || op2->st_type == 'S');
    }
    if(op1->st_type == 'b'){ // Bool
        return (op2->st_type == 'b');
    }
    return false;
}

/**
 * @brief Zistí, či sú dátové typy operandov kompatibilné pre "??"
 * @returns true ak sú kompatibilné, inak false
**/
bool are_compatible_n(ptoken_T *op1, ptoken_T *op2){

    if(op1->type == INT_CONST || op1->st_type == 'i' || op1->st_type == 'I'){ // Integer
        return (op2->type == INT_CONST || op2->st_type == 'i' || op2->st_type == 'I');
    }
    if(op1->type == DOUBLE_CONST || op1->st_type == 'd' || op1->st_type == 'D'){ // Double
        return (op2->type == DOUBLE_CONST || op2->st_type == 'd' || op2->st_type == 'D');
    }
    if(op1->type == STRING_CONST || op1->st_type == 's' || op1->st_type == 'S'){ // String
        return (op2->type == STRING_CONST || op2->st_type == 's' || op2->st_type == 'S');
    }

    return false;
}
/**
 * @brief Zistí, či je dátový typy operandu nil alebo môže byť nil
 * @returns true ak je nil alebo môže byť nil, false ak nie
**/
bool is_nil_type(ptoken_T *op){
    return (op->st_type == 'I' || op->st_type == 'D' || 
    op->st_type == 'S' || op->st_type == 'N');
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

/******************************************************************************************
 *Ostatné funkcie
*****************************************************************************************/


/**
 * @brief Funkcia zavolaná pred ukončením parseExpression
 * @details Uvoľní alokovanú pamäť pomocných premenných a zásobníku
**/
void endParse_syn(stack_t *stack, stack_t *postfixExpr){
    stack_dispose(stack);
    stack_dispose(postfixExpr);
}
/**
 * @brief Funkcia zavolaná pred ukončením parseExpression
 * @details Uvoľní alokovanú pamäť pomocných premenných a zásobníku
**/
void endParse_sem(stack_t *stack, stack_t *postfixExpr){
    stack_clear_free(stack);
    stack_dispose(postfixExpr);
}

/**
 * @brief Konvertuje infixový výraz do postfixovej formy
 * @details Volaná počas syntaktickej analýzy výrazu pri každom tokene
 * @returns 0 ak prebehlo vloženie tokenu úspešne, inak konkrétny chybový kód
**/
int infix2postfix(stack_t *stack, stack_t *postfixExpr, token_T *infix_token){
    
    int status = 0; // Pomocná premenná pre uloženie návratového kódu funkcií

    if(infix_token == NULL) // Koniec výrazu => všetky prvky zo zásobníka vkladáme do postfixexpression
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
		return stack_push_token(postfixExpr, infix_token); // Vloženie operandu do postfixového výrazu, koniec funkcie
	}
    return 0;
}

void copy_data(ptoken_T *source, ptoken_T *destination){

    destination->init = source->init;
    destination->st_type = source->st_type;
    destination->type = source->type;
    destination->codename = source->codename; // Vloženie reťazca
    destination->id = source->id; // Vloženie reťazca
}





/******************************************************************************************
 *Hlavná funkcia
*****************************************************************************************/

int parseExpression(char* result_type) {

    stack_t stack; // Zásobník pre konverziu výrazu na postfixovú formu
    stack_t postfixExpr; // Zásobník pre uloženie postfixového výrazu
    stack_init(&stack); // Inicializácia zásobníka
    stack_init(&postfixExpr); // Inicializácia zásobníka

    int prevTokenType = NO_PREV; // Pomocná premenná pre uloženie typu tokenu pred momentálne spracovaným
    int bracketCount = 0; // Premenná na overenie korektnosti zátvoriek "()" vo výraze
    int status = 0; // Premenná na overenie priebehu volania funkcie
    //printf("Infix: ");
//==============================Syntaktická analýza============================
    while(true) // Pokým sa nespracuje celý výraz
    {

        if(!valid_token_type(tkn->type)) // Ak token nemôže patriť do výrazu 
        {
            if(tkn->type == 0) // Token je typu INVALID
            {
                endParse_syn(&stack, &postfixExpr); // Upratanie pred skončením funkcie
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
                
                if(infix2postfix(&stack, &postfixExpr, NULL) == COMPILER_ERROR){ // Signalizuje ukončenie postfix výrazu
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
                        if(infix2postfix(&stack, &postfixExpr, NULL) == COMPILER_ERROR){ // Signalizuje ukončenie postfix výrazu
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
        

        //printf("Token %s\n", StrRead(&tkn->atr));
        status = infix2postfix(&stack, &postfixExpr, tkn); // Pridanie tokenu do postfix výrazu
        if(status != 0) // Pridanie tokenu do postfix výrazu nebolo úspešné
        {
            endParse_syn(&stack, &postfixExpr); // Upratenie pred ukončením
            return status; // Vrátenie chybového kódu            
        }

        //printf("%s ",tkn->atr.data);
        
        prevTokenType = tkn->type; // Uloženie typu predošlého tokenu
        status = nextToken(); // Požiadanie o ďalší token z výrazu
        if(status != 0){ // nextToken vrátil chybu
            return status; // Vrátenie chybovej hodnoty
        }

    }

    if(prevTokenType == NO_PREV) // Symbolizuje chybnú syntax
    {
        endParse_syn(&stack, &postfixExpr); // Upratanie pred skončením funkcie
        return SYN_ERR; // Vrátenie chybového stavu
    }

    //printf("\nPostfix: ");
    //int i=0;
    //while(i<postfixExpr.size){
    //    printf("%s ",postfixExpr.array[i]->id.data);
    //    i++;
    //}
    //printf("\n");

//==============================Sémantická analýza============================
    ptoken_T *var_a, *var_b; // Pomocné premenné pre sémantickú analýzu

    for(int index = 0; index<postfixExpr.size; index++) // Kým sa nespracuje celý postfix výraz
    {
        if(is_operand(postfixExpr.array[index]->type)) // Operand
        {
            //printf("Token %s je operand s typom %c\n", StrRead(&postfixExpr.array[index]->id),postfixExpr.array[index]->st_type );
            ptoken_T *new_token = malloc(sizeof(ptoken_T)); // Vytvorenie nového tokenu kvôli zachovaniu hodnôt v pôvodnom
            copy_data(postfixExpr.array[index], new_token); // Skopírovanie hodnôt z pôvodného tokenu
            //printf("Novy token id:%s, type:%c\n", StrRead(&new_token->id),new_token->type);
            if(stack_push_ptoken(&stack, new_token) == COMPILER_ERROR) // Operand sa vloží na zásobník
            {
                endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                return COMPILER_ERROR; // Vrátenie chybového stavu
            }

            genCode("PUSHS",StrRead(&(postfixExpr.array[index]->codename)),NULL, NULL); // Vloženie premennej na zásobník
        }
        if(is_binary_operator(postfixExpr.array[index]->type)) // Binárny operátor
        {
            //printf("Token %s je bin operator\n", StrRead(&postfixExpr.array[index]->id) );
            var_b = stack_top(&stack);
            stack_pop(&stack);
            var_a = stack_top(&stack);
            stack_pop(&stack);
            // Popneme 2 premenné zo zásobníka

            if(is_arithmetic_operator(postfixExpr.array[index]->type)) // Aritmetický operátor
            {
                //printf("Token %s je aritmeticky operator\n", StrRead(&postfixExpr.array[index]->id) );
                if(var_a->st_type == 'I' || var_a->st_type == 'D' || var_a->st_type == 'S' || 
                var_a->st_type == 'N' || var_b->st_type == 'I' || var_b->st_type == 'D' || 
                var_b->st_type == 'S' || var_b->st_type == 'N')
                { // Jeden z operandov je nil alebo hodnota ktorá môže obsahovať nil
                    //printf("Jeden z operandov je nil hodnota\n");
                    free(var_a); // Vymazanie tokenu
                    free(var_b); // Vymazanie tokenu
                    endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                    return SEM_ERR_TYPE;
                }
                if(var_a->st_type == 's' || var_a->type == STRING_CONST) // Prvý operand je reťazec
                {
                    if(postfixExpr.array[index]->type == OP_PLUS) // Operátor je "+"
                    {
                        if(var_b->st_type == 's' || var_b->type == STRING_CONST) // Druhý operand je tiež reťazec
                        {
                            var_a->st_type = 's'; // Výsledok konkatenácie je typu string
                            free(var_b); // Vymazanie tokenu
                            if(stack_push_ptoken(&stack, var_a) != 0){ // Vloženie tokenu na zásobník
                                endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                                return COMPILER_ERROR;
                            }

                            genCode("POPS","GF@!tmp1", NULL, NULL); // Popnutie reťazca do pomocnej premennej
                            genCode("POPS","GF@!tmp2", NULL, NULL); // Popnutie reťazca do pomocnej premennej
                            genCode("CONCAT", "GF@!tmp3", NULL, NULL); // Konkatenácia reťazcov
                            genCode("PUSHS", "GF@!tmp3", NULL, NULL); // Pushnutie konkatenovaného reťazca na stack

                            continue; // Posúvame sa na ďalší znak v postfix výraze
                        }
                        else{
                            free(var_a); // Vymazanie tokenu
                            free(var_b); // Vymazanie tokenu
                            endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                            return SEM_ERR_TYPE;
                        }
                    }
                    else{
                        free(var_a); // Vymazanie tokenu
                        free(var_b); // Vymazanie tokenu
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return SEM_ERR_TYPE;
                    }
                }
                if((var_a->st_type == 'i' || var_a->type == INT_CONST) && (var_b->st_type == 'i' || var_b->type == INT_CONST)){ // 2 Inty
                    
                    var_a->st_type = 'i'; // Výsledok operácie je typu int
                    free(var_b); // Vymazanie tokenu
                    if(stack_push_ptoken(&stack, var_a) != 0){ // Vloženie tokenu na zásobník
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                    }
                    switch (postfixExpr.array[index]->type){
                    case OP_PLUS:
                        genCode("ADDS",NULL, NULL, NULL); // Sčítanie hodnôt na vrchole zásobníka
                        break;
                    case OP_MINUS:
                        genCode("SUBS",NULL, NULL, NULL); // Odčítanie hodnôt na vrchole zásobníka
                        break;
                    case OP_DIV:
                        genCode("MULS",NULL, NULL, NULL); // Podiel hodnôt na vrchole zásobníka
                        break;
                    case OP_MUL:
                        genCode("DIVS",NULL, NULL, NULL); // Vynásobenie hodnôt na vrchole zásobníka
                        break;
                    }

                    continue; // Posúvame sa na ďalší znak v postfix výraze
                }
                if((var_a->st_type == 'd' || var_a->type == DOUBLE_CONST) && (var_b->st_type == 'd' || var_b->type == DOUBLE_CONST)){ // 2 Doubly
                    
                    //printf("2 double\n");
                    var_a->st_type = 'd'; // Výsledok operácie je typu double
                    if(stack_push_ptoken(&stack, var_a) != 0){ // Vloženie tokenu na zásobník
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                    }
                    free(var_b); // Vymazanie tokenu

                    switch (postfixExpr.array[index]->type){
                    case OP_PLUS:
                        genCode("ADDS",NULL, NULL, NULL); // Sčítanie hodnôt na vrchole zásobníka
                        break;
                    case OP_MINUS:
                        genCode("SUBS",NULL, NULL, NULL); // Odčítanie hodnôt na vrchole zásobníka
                        break;
                    case OP_DIV:
                        genCode("MULS",NULL, NULL, NULL); // Podiel hodnôt na vrchole zásobníka
                        break;
                    case OP_MUL:
                        genCode("DIVS",NULL, NULL, NULL); //  Vynásobenie hodnôt na vrchole zásobníka
                        break;
                    }
                    continue; // Posúvame sa na ďalší znak v postfix výraze
                }
                if((var_a->type == INT_CONST && (var_b->type == DOUBLE_CONST || var_b->st_type == 'd'))|| 
                (var_b->type == INT_CONST && (var_a->type == DOUBLE_CONST || var_a->st_type == 'd'))) // Int konštanta a Double
                { 
                    if(var_a->st_type == INT_CONST){ // Int konštanta je na zásobníku druhá z vrchu
                        genCode("POPS","GF@!tmp2", NULL, NULL); // Popnutie double premennej
                        genCode("POPS","GF@!tmp1", NULL, NULL); // Popnutie int premennej
                    }
                    else{ // Int konštanta je na vrchole zásobníku
                        genCode("POPS","GF@!tmp1", NULL, NULL); // Popnutie int premennej
                        genCode("POPS","GF@!tmp2", NULL, NULL); // Popnutie double premennej
                    }
                    genCode("INT2FLOAT", "GF@!tmp3", "GF@!tmp1", NULL); // Konverzia int na double

                    var_a->st_type = 'd'; // Výsledok operácie je typu double
                    free(var_b); // Vymazanie tokenu
                    if(stack_push_ptoken(&stack, var_a) != 0){ // Vloženie tokenu na zásobník
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                    }

                    switch (postfixExpr.array[index]->type){
                    case OP_PLUS:
                        genCode("ADD","GF@!tmp1", "GF@!tmp3", "GF@!tmp2"); // Sčítanie hodnôt
                        break;
                    case OP_MINUS:
                        genCode("SUB","GF@!tmp1", "GF@!tmp3", "GF@!tmp2"); // Odčítanie hodnôt
                        break;
                    case OP_DIV:
                        genCode("MUL","GF@!tmp1", "GF@!tmp3", "GF@!tmp2"); // Podiel hodnôt
                        break;
                    case OP_MUL:
                        genCode("DIV","GF@!tmp1", "GF@!tmp3", "GF@!tmp2"); // Vynásobenie hodnôt
                        break;
                    }
                    genCode("PUSHS", "GF@!tmp1", NULL, NULL); // Pushnutie výsledku na stack

                    continue; // Posúvame sa na ďalší znak v postfix výraze

                }
                else{ // Typy nie sú kompatibilné
                    free(var_a); // Vymazanie tokenu
                    free(var_b); // Vymazanie tokenu
                    endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                    return SEM_ERR_TYPE;
                }
            }

            if(is_logical_operator(postfixExpr.array[index]->type)) // Logický operátor
            {
                //printf("Token %s je logicky operator\n", StrRead(&postfixExpr.array[index]->id) );
                
                if(are_compatible_l(var_a, var_b)) // Overenie, či sú dátové typy kompatibilné pre logickú operáciu
                {
                    if(var_a->st_type == 'b' && var_b->st_type == 'b' &&
                    (postfixExpr.array[index]->type != EQ && postfixExpr.array[index]->type != NEQ)){
                    // Bool operandy môžu byť porovnané iba operátorom "==" alebo "!="
                        free(var_a); // Vymazanie tokenu
                        free(var_b); // Vymazanie tokenu
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return SEM_ERR_TYPE;
                    }

                    var_a->st_type = 'b'; // Výsledný token bude typu boolean
                    if(stack_push_ptoken(&stack, var_a) != 0){ // Pushnutie nového tokenu na stack
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                    }
                    free(var_b); // Vymazanie druhého tokenu

                    switch (postfixExpr.array[index]->type){
                    case EQ:
                        genCode("EQS",NULL, NULL, NULL); // Rovnosť hodnôt
                        break;
                    case NEQ:
                        genCode("EQS",NULL, NULL, NULL); // Rovnosť hodnôt
                        genCode("NOTS",NULL, NULL, NULL); // => nerovnosť hodnôt
                        break;
                    case GT:
                        genCode("GTS",NULL, NULL, NULL); // A > B
                        break;
                    case LT:
                        genCode("DIV",NULL, NULL, NULL); // A<B
                        break;
                    case LTEQ:
                        genCode("GT",NULL, NULL, NULL); // A > B
                        genCode("NOTS",NULL, NULL, NULL); // A <= B
                        break;
                    case GTEQ:
                        genCode("LT",NULL, NULL, NULL); // A < B
                        genCode("NOTS",NULL, NULL, NULL); // A >= B
                        break;
                    }
                    
                    continue; // Posúvame sa na ďalší znak v postfix výraze
                }
                else{
                    free(var_a); // Vymazanie tokenu
                    free(var_b); // Vymazanie tokenu
                    endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                    return SEM_ERR_TYPE;
                }
            }
            if(postfixExpr.array[index]->type == TEST_NIL) // Test nil hodnoty "??"
            {
                if(is_nil_type(var_b)) // Druhý operand je nil alebo nil typ
                {
                    //printf("Druhý operand je nil alebo nil typ\n");
                    free(var_a); // Vymazanie tokenu
                    free(var_b); // Vymazanie tokenu
                    endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                    return SEM_ERR_TYPE;
                }
                if(var_a->st_type == 'N')// Prvý operand je nil
                {
                    //printf("Prvý operand je nil\n");
                    if(stack_push_ptoken(&stack, var_b) != 0){ // Pushnutie druhého tokenu na stack
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                    }
                    free(var_a); // Vymazanie prvého tokenu
                    genCode("POPS","GF@!tmp1", NULL, NULL); // Odstránenie nil zo zásobníka
                    genCode("POPS","GF@!tmp2", NULL, NULL); // Popnutie non-nil premennej do pomocnej premennej
                    genCode("PUSHS","GF@!tmp1", NULL, NULL); // Vrátenie non-nil premennej späť na zásobník
                    continue; // Posúvame sa na ďalší token
                }
                if(are_compatible_n(var_a, var_b)) // Ak majú tokeny kompatibilný dátový typ
                {
                    //printf("tokeny kompatibilný dátový typ\n");
                    if(!is_nil_type(var_a)) // Prvý operand nikdy nebude nil => je výsledok výrazu
                    {
                        //printf("Prvý operand nikdy nebude nil\n");
                        if(stack_push_ptoken(&stack, var_a) != 0){ // Pushnutie druhého tokenu na stack
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                        }
                        free(var_b); // Vymazanie druhého tokenu
                        genCode("POPS","GF@!tmp1", NULL, NULL); // Odstránenie nil zo zásobníka
                        continue;
                    }
                    else // Prvý operand môže byť nil
                    {
                        //printf("Prvý operand môže byť nil\n");
                        if(stack_push_ptoken(&stack, var_b) != 0){ // Pushnutie druhého tokenu na stack
                        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                        return COMPILER_ERROR;
                        }
                        free(var_a); // Vymazanie druhého tokenu

                        str_T label1, label2;
                        StrInit(&label1);
                        StrInit(&label2);

                        genUniqLabel("testnil","l1",&label1); // Vygenerovanie labelu pre podmienený skok
                        genUniqLabel("testnil","l2",&label2); // Vygenerovanie labelu pre podmienený skok

                        genCode("POPS","GF@!tmp2", NULL, NULL); // Popnutie non-nil premennej do pomocnej premennej
                        genCode("POPS","GF@!tmp1", NULL, NULL); // Popnutie possible-nil premennej do pomocnej premennej
                        genCode("JUMPIFEQ", StrRead(&label1),"GF@!tmp1", "nil@nil"); // Ak sa prvá premenná rovná nil, skok na náveštie 1
                        genCode("PUSHS","GF@!tmp1", NULL, NULL); // V tomto prípade prvá premenná nie je nil, pushnutie prvej premennej na zásobník
                        genCode("JUMP", StrRead(&label2), NULL, NULL); // Skok na koniec funkcie
                        genCode("LABEL", StrRead(&label1), NULL, NULL); // Náveštie 1
                        genCode("PUSHS","GF@!tmp2", NULL, NULL); // V tomto prípade prvá premenná je nil, pushnutie 2. premennej na zásobník
                        genCode("LABEL", StrRead(&label2), NULL, NULL); // Náveštie 2 - koniec funkcie

                        StrDestroy(&label1);
                        StrDestroy(&label2);
                        continue;
                    }


                }
                
            }
        }
        
        if(postfixExpr.array[index]->type == EXCL) // Výkričník
        {
            //printf("Token %s je vykricnik\n", StrRead(&postfixExpr.array[index]->id) );
            //printf("Token %s ma typ %c\n", StrRead(&stack_top(&stack)->id), stack_top(&stack)->st_type);
            if(stack_top(&stack)->st_type == 'I'){ // typ Int? 
                stack_top(&stack)->st_type = 'i'; // pretypovanie na Int
            }
            if(stack_top(&stack)->st_type == 'D'){ // typ Double?
                stack_top(&stack)->st_type = 'd'; // pretypovanie na Double
            }
            if(stack_top(&stack)->st_type == 'S'){ // typ String?
                stack_top(&stack)->st_type = 's'; // pretypovanie na String
            }
            if(stack_top(&stack)->type == NIL){ // Výraz "nil!"
                free(var_a);
                free(var_b);
                endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
                return SEM_ERR_OTHER;
            }
            //printf("Token %s ma novy typ %c\n", StrRead(&stack_top(&stack)->id), stack_top(&stack)->st_type);
            // Pre konštanty operátor "!" nemá efekt
        }
    }
    if(stack.size == 1){ // Výsledný typ je na vrchole zásobníka
        *result_type = stack_top(&stack)->st_type; // Zapísanie výsledného typu výrazu
        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie

        return COMPILATION_OK; // Úspešný koniec
    }
    else{
        endParse_sem(&stack, &postfixExpr); // Upratanie pred skončením funkcie
        return SEM_ERR_OTHER;
    }
    


    
    
    return COMPILATION_OK;
}


/* Koniec súboru exp.c */
