# IFJ23 Compiler

Implementation of a compiler for the **IFJ23 language**, a simplified subset of **Swift 5**. The compiler performs all standard stages: lexical analysis, syntactic analysis, semantic analysis, and code generation targeting the **IFJcode23 intermediate language**.

##  Development Team


| Member              | Contribution |
|---------------------|--------------|
| Boris Hatala        | Symbol table, string utils, helper functions |
| František Holáň     | Lexical analyzer, helper functions |
| Michal Krulich      | Team lead, parser (top-down), testing, code generation |
| Stanislav Letaši    | Expression parser (bottom-up), documentation |


##  Project Structure

| File(s)              | Description |
|----------------------|-------------|
| `main.c`             | Main entry point, start of parsing |
| `scanner.[h/c]`      | Lexical analyzer (tokenizer) using a deterministic finite automaton |
| `parser.[h/c]`       | Syntactic and semantic analysis, recursive descent and precedence parsing |
| `exp.[h/c]`          | Expression parsing and code generation using precedence analysis |
| `generator.[h/c]`    | Code generator for IFJcode23 |
| `symtable.[h/c]`     | Symbol table implemented as chained hash tables |
| `strR.[h/c]`         | Dynamic string with automatic resizing |
| `dll.[h/c]`          | Double-linked list used for instruction queues |
| `decode.[h/c]`       | String escape sequence decoding for IFJcode23 |
| `logErr.[h/c]`        | Error logging and reporting |

## Implementation Details

### Lexical Analyzer
- Implemented as a deterministic finite automaton inside `getToken()`
- Tokenizes keywords, identifiers, literals, operators, and handles escape sequences
- Identifies 42 token types and tracks line/column numbers
- Uses a fallback lookup to differentiate between identifiers and keywords

### Parser
- Recursive descent based on a top-down LL grammar
- Uses a fallback LL table and a dedicated bottom-up precedence parser for expressions
- Grammar is split for function definitions, statements, blocks, expressions, and control flow

### Symbol Table
- Implemented using chained hash tables with implicit chaining
- Supports local and global scopes using a doubly-linked list of symbol table blocks
- Manages identifiers, types, initialization status, and function signatures
- Hashing uses djb2 and a modified secondary hash for collision resolution

### Code Generation
- Code is generated during parsing and stored in linked instruction lists
- Distinguishes between code for main and for user-defined functions
- Ensures label and variable uniqueness using counters
- Supports function definitions, calls, conditional and loop constructs
- Handles built-in functions like `substr`, `print`, `inputs`, `inputi`, and `inputf`

##  Installation and Usage

This project requires a C compiler (e.g., `gcc`) and a Unix-like environment. To compile, simply run `make` in the root directory.

Run the compiler with a source file: 
```
./compiler < source.ifj23 > output.ifjcode23

Input: IFJ23 source code via stdin
Output: IFJcode23 intermediate code via stdout
```

##  Context-Free Grammar Rules Used by the Parser

The parser follows the LL grammar below.  
Legend:  
- `ε` = epsilon (empty string)  
- `id` = identifier  
- `exp` = expression  

### Statements

```
1.  <STAT> -> ε
2.  <STAT> -> let id <DEF_VAR> <STAT>        // e.g., let a : Int = 4
3.  <STAT> -> var id <DEF_VAR> <STAT>
10. <STAT> -> id = <ASSIGN> <STAT>           // assignment
11. <STAT> -> { <STAT> } <STAT>              // code block
12. <STAT> -> id ( <PAR_LIST> ) <STAT>       // function call
20. <STAT> -> func id ( <FN_SIG> ) <FN_RET_TYPE> { <STAT> } <STAT> // function definition
32. <STAT> -> return <RET_VAL> <STAT>        // return statement
35. <STAT> -> if <COND> { <STAT> } else { <STAT> } <STAT> // if-else
38. <STAT> -> while exp { <STAT> } <STAT>    // while loop
```

### Variable Definition

```
4.  <DEF_VAR> -> : <TYPE> <INIT_VAL>
5.  <DEF_VAR> -> = <ASSIGN>
6.  <INIT_VAL> -> = <ASSIGN>
7.  <INIT_VAL> -> ε
```

### Assignment

```
8.  <ASSIGN> -> exp
9.  <ASSIGN> -> id ( <PAR_LIST> )
```

### Function Call Parameters

```
13. <PAR_LIST> -> id : term <PAR_IN_NEXT>
14. <PAR_LIST> -> term <PAR_IN_NEXT>
15. <PAR_LIST> -> ε
16. <PAR_IN_NEXT> -> , <PAR_IN> <PAR_IN_NEXT>
17. <PAR_IN_NEXT> -> ε
18. <PAR_IN> -> id : term
19. <PAR_IN> -> term
```

### Function Signature

```
21. <FN_SIG> -> id id : <TYPE> <FN_PAR_NEXT>
22. <FN_SIG> -> _ id : <TYPE> <FN_PAR_NEXT>
23. <FN_SIG> -> ε
24. <FN_PAR_NEXT> -> , <FN_PAR> <FN_PAR_NEXT>
25. <FN_PAR_NEXT> -> ε
26. <FN_PAR> -> id id : <TYPE>
27. <FN_PAR> -> _ id : <TYPE>
28. <FN_PAR> -> id _ : <TYPE>
29. <FN_PAR> -> _ _ : <TYPE>
```

### Function Return Type

```
30. <FN_RET_TYPE> -> -> <TYPE>
31. <FN_RET_TYPE> -> ε
```

### Return Value

```
33. <RET_VAL> -> exp
34. <RET_VAL> -> ε
```

### Condition

```
36. <COND> -> exp
37. <COND> -> let id
```

### Data Types

```
39. <TYPE> -> Integer <QUESTMARK>
40. <TYPE> -> Double <QUESTMARK>
41. <TYPE> -> String <QUESTMARK>
42. <QUESTMARK> -> ?
43. <QUESTMARK> -> ε
```