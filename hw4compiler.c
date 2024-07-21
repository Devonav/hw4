/*
July 7, 2024
COP 3402 Systems Software Assignment 3
This program is written by: Devon Villalona and Izaac Plambeck
HW4: Pl-0 Tiny complier with Procedures and Call Statements
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_IDENT_LEN 11
#define MAX_NUM_LEN 5
#define MAX_LEXEMES 1000
#define MAX_SYMBOL_TABLE_SIZE 500
#define MAX_CODE_LENGTH 500
#define MAX_LEVEL 3
#define PROC_SPACE 4 // Space required for procedure calls

// token_type enum
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, fisym, eqlsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym,
    whilesym, dosym, callsym, constsym, varsym, procsym, writesym,
    readsym, elsesym, errorsym
} token_type;

typedef struct {
    char lexeme[MAX_IDENT_LEN + 1];
    token_type token;
    char error[50];
} Lexeme;

typedef struct {
    int kind; // const = 1, var = 2, proc = 3
    char name[MAX_IDENT_LEN + 1];
    int val;
    int level;
    int addr;
    int mark;
} symbol;

typedef struct {
    char op[4];
    int l;
    int m;
} instruction;

// Global variables
Lexeme lexemes[MAX_LEXEMES];
int lexeme_count = 0;
int current_token = 0;
symbol symbol_table[MAX_SYMBOL_TABLE_SIZE];
int symbol_count = 0;
instruction code[MAX_CODE_LENGTH];
int code_index = 0;
int current_level = 0;
char *reservedWords[] = {
    "const", "var", "procedure", "call", "begin", "end", "if", "fi",
    "then", "else", "while", "do", "read", "write"
};
token_type reservedTokens[] = {
    constsym, varsym, procsym, callsym, beginsym, endsym, ifsym, fisym,
    thensym, elsesym, whilesym, dosym, readsym, writesym
};

// Function prototypes
void addLexeme(char *lex, token_type token, char *error);
token_type identifyReservedWord(char *word);
void processInput(const char *input);
void readInputFile(const char *filename, char *buffer, int size);
void writeOutputFile(const char *filename);
void writeElfFile(const char *filename);
void program();
void block(int level, int tx);
void const_declaration();
int var_declaration();
void procedure_declaration(int level, int *tx, int *dx);
void statement();
void condition();
void expression();
void term();
void factor();
int symbol_table_check(const char *name);
void add_symbol(int kind, const char *name, int val, int level, int addr);
void emit(const char *op, int l, int m);
void error(const char *message, const char *detail);
void mark_all_symbols();
void displayInput(const char *input);
void displayGeneratedCode();

// Function to add lexeme to the lexeme list
void addLexeme(char *lex, token_type token, char *error) {
    if (lexeme_count < MAX_LEXEMES) {
        strcpy(lexemes[lexeme_count].lexeme, lex);
        lexemes[lexeme_count].token = token;
        if (error) {
            strcpy(lexemes[lexeme_count].error, error);
        } else {
            lexemes[lexeme_count].error[0] = '\0';
        }
        lexeme_count++;
    }
}

// Function to identify reserved words
token_type identifyReservedWord(char *word) {
    for (int i = 0; i < sizeof(reservedWords) / sizeof(reservedWords[0]); i++) {
        if (strcmp(word, reservedWords[i]) == 0) {
            return reservedTokens[i];
        }
    }
    return identsym;
}

// Function to process the input
void processInput(const char *input) {
    int length = strlen(input);
    char buffer[MAX_IDENT_LEN + 1];
    int buffer_index = 0;

    for (int i = 0; i < length; i++) {
        char c = input[i];

        if (isspace(c)) continue;

        if (isalpha(c)) {
            buffer_index = 0;
            while (isalnum(c) && buffer_index < MAX_IDENT_LEN) {
                buffer[buffer_index++] = c;
                c = input[++i];
            }
            buffer[buffer_index] = '\0';
            if (isalnum(c)) {
                while (isalnum(c)) c = input[++i];
                addLexeme(buffer, errorsym, "Name too long");
            } else {
                token_type token = identifyReservedWord(buffer);
                addLexeme(buffer, token, NULL);
            }
            i--;
        } else if (isdigit(c)) {
            buffer_index = 0;
            while (isdigit(c) && buffer_index < MAX_NUM_LEN) {
                buffer[buffer_index++] = c;
                c = input[++i];
            }
            buffer[buffer_index] = '\0';
            if (isdigit(c)) {
                while (isdigit(c)) c = input[++i];
                addLexeme(buffer, errorsym, "Number too long");
            } else {
                addLexeme(buffer, numbersym, NULL);
            }
            i--;
        } else {
            buffer[0] = c;
            buffer[1] = '\0';
            switch (c) {
                case '+': addLexeme(buffer, plussym, NULL); break;
                case '-': addLexeme(buffer, minussym, NULL); break;
                case '*': addLexeme(buffer, multsym, NULL); break;
                case '/':
                    if (input[i + 1] == '*') {
                        i += 2;
                        while (i < length - 1 && !(input[i] == '*' && input[i + 1] == '/')) {
                            i++;
                        }
                        if (i < length - 1) {
                            i++;
                        } else {
                            addLexeme("/*", errorsym, "Unterminated comment");
                        }
                    } else {
                        addLexeme(buffer, slashsym, NULL);
                    }
                    break;
                case '=': addLexeme(buffer, eqlsym, NULL); break;
                case '<':
                    if (input[i + 1] == '>') {
                        buffer[1] = '>';
                        buffer[2] = '\0';
                        addLexeme(buffer, neqsym, NULL);
                        i++;
                    } else if (input[i + 1] == '=') {
                        buffer[1] = '=';
                        buffer[2] = '\0';
                        addLexeme(buffer, leqsym, NULL);
                        i++;
                    } else {
                        addLexeme(buffer, lessym, NULL);
                    }
                    break;
                case '>':
                    if (input[i + 1] == '=') {
                        buffer[1] = '=';
                        buffer[2] = '\0';
                        addLexeme(buffer, geqsym, NULL);
                        i++;
                    } else {
                        addLexeme(buffer, gtrsym, NULL);
                    }
                    break;
                case ':':
                    if (input[i + 1] == '=') {
                        buffer[1] = '=';
                        buffer[2] = '\0';
                        addLexeme(buffer, becomessym, NULL);
                        i++;
                    } else {
                        addLexeme(buffer, errorsym, "Invalid character");
                    }
                    break;
                case ';': addLexeme(buffer, semicolonsym, NULL); break;
                case ',': addLexeme(buffer, commasym, NULL); break;
                case '.': addLexeme(buffer, periodsym, NULL); break;
                case '(': addLexeme(buffer, lparentsym, NULL); break;
                case ')': addLexeme(buffer, rparentsym, NULL); break;
                default:
                    addLexeme(buffer, errorsym, "Invalid character");
                    break;
            }
        }
    }
}

// Function to read the input file
void readInputFile(const char *filename, char *buffer, int size) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    int index = 0;
    char c;
    while ((c = fgetc(file)) != EOF && index < size - 1) {
        buffer[index++] = c;
    }
    buffer[index] = '\0';
    fclose(file);
}

// Function to write the output file
void writeOutputFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    fprintf(file, "Line OP L M\n");
    for (int i = 0; i < code_index; i++) {
        fprintf(file, "%d %s %d %d\n", i, code[i].op, code[i].l, code[i].m);
    }

    fclose(file);
}

// Function to write the elf file
void writeElfFile(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < code_index; i++) {
        int op = 0;
        if (strcmp(code[i].op, "JMP") == 0) op = 7;
        else if (strcmp(code[i].op, "INC") == 0) op = 6;
        else if (strcmp(code[i].op, "LIT") == 0) op = 1;
        else if (strcmp(code[i].op, "STO") == 0) op = 4;
        else if (strcmp(code[i].op, "LOD") == 0) op = 3;
        else if (strcmp(code[i].op, "SYS") == 0) op = 9;
        else if (strcmp(code[i].op, "ADD") == 0) op = 2;
        else if (strcmp(code[i].op, "SUB") == 0) op = 2;
        else if (strcmp(code[i].op, "MUL") == 0) op = 2;
        else if (strcmp(code[i].op, "DIV") == 0) op = 2;
        else if (strcmp(code[i].op, "EQL") == 0) op = 2;
        else if (strcmp(code[i].op, "NEQ") == 0) op = 2;
        else if (strcmp(code[i].op, "LSS") == 0) op = 2;
        else if (strcmp(code[i].op, "LEQ") == 0) op = 2;
        else if (strcmp(code[i].op, "GTR") == 0) op = 2;
        else if (strcmp(code[i].op, "GEQ") == 0) op = 2;
        else if (strcmp(code[i].op, "JPC") == 0) op = 8;
        else if (strcmp(code[i].op, "ODD") == 0) op = 2;

        fprintf(file, "%d %d %d\n", op, code[i].l, code[i].m);
    }

    fclose(file);
}

// Function to handle errors
void error(const char *message, const char *detail) {
    if (detail) {
        fprintf(stderr, "Error: %s %s\n", message, detail);
    } else {
        fprintf(stderr, "Error: %s\n", message);
    }
    exit(EXIT_FAILURE);
}

// Function to display the input program
void displayInput(const char *input) {
    printf("Input Program:\n%s\n", input);
}

// Function to display the generated code
void displayGeneratedCode() {
    printf("Generated Code (Assembly):\n");
    for (int i = 0; i < code_index; i++) {
        printf("%d %s %d %d\n", i, code[i].op, code[i].l, code[i].m);
    }
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const int bufferSize = 10000;
    char input[bufferSize];

    readInputFile(argv[1], input, bufferSize);
    displayInput(input);
    processInput(input);

    program();

    printf("No errors, program is syntactically correct\n");

    displayGeneratedCode();
    writeOutputFile("output.txt");
    writeElfFile("elf.txt");

    return 0;
}

// Function to check the symbol table
int symbol_table_check(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        printf("Debug: Checking symbol %s against %s (kind: %d, marked: %d)\n", name, symbol_table[i].name, symbol_table[i].kind, symbol_table[i].mark);
        if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].mark == 0) {
            return i;
        }
    }
    return -1;
}

// Function to add a symbol to the symbol table
void add_symbol(int kind, const char *name, int val, int level, int addr) {
    if (symbol_count >= MAX_SYMBOL_TABLE_SIZE) {
        error("Symbol table overflow", NULL);
    }
    symbol_table[symbol_count].kind = kind;
    strncpy(symbol_table[symbol_count].name, name, MAX_IDENT_LEN);
    symbol_table[symbol_count].val = val;
    symbol_table[symbol_count].level = level;
    symbol_table[symbol_count].addr = addr;
    symbol_table[symbol_count].mark = 0;
    symbol_count++;
}

// Function to emit code
void emit(const char *op, int l, int m) {
    if (code_index >= MAX_CODE_LENGTH) {
        error("Code size exceeded", NULL);
    }
    strncpy(code[code_index].op, op, 4);
    code[code_index].l = l;
    code[code_index].m = m;
    code_index++;
}

// Function to mark all symbols
void mark_all_symbols() {
    for (int i = 0; i < symbol_count; i++) {
        symbol_table[i].mark = 1;
    }
}

void program() {
    emit("JMP", 0, 3);
    block(0, 0);
    if (lexemes[current_token].token != periodsym) {
        error("program must end with period", NULL);
    }
    emit("SYS", 0, 3);
    mark_all_symbols();
}

// Block is a helper function for program that handles the base case of the recursion and the different types of blocks that can be declared
void block(int level, int tx) {
    int dx = 3, tx0 = tx, cx0 = code_index;
    if (level > MAX_LEVEL) {
        error("maximum block level exceeded", NULL);
    }
    symbol_table[tx].addr = code_index;
    emit("JMP", 0, 0);
    const_declaration();
    int numVars = var_declaration();
    while (lexemes[current_token].token == procsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error("procedure must be followed by identifier", NULL);
        }
        add_symbol(3, lexemes[current_token].lexeme, 0, level, code_index);
        current_token++;
        if (lexemes[current_token].token != semicolonsym) {
            error("semicolon expected after procedure declaration", NULL);
        }
        current_token++;
        block(level + 1, tx);
        if (lexemes[current_token].token != semicolonsym) {
            error("semicolon expected after procedure block", NULL);
        }
        current_token++;
    }
    code[symbol_table[tx0].addr].m = code_index;
    emit("INC", level, dx + numVars);
    statement();
    emit("OPR", 0, 0);
}



// const_declaration is a helper function for block that handles the declaration of constants
void const_declaration() {
    if (lexemes[current_token].token == constsym) {
        do {
            current_token++;
            if (lexemes[current_token].token != identsym) {
                error("const must be followed by identifier", NULL);
            }
            if (symbol_table_check(lexemes[current_token].lexeme) != -1) {
                error("symbol name has already been declared", lexemes[current_token].lexeme);
            }
            char name[MAX_IDENT_LEN + 1];
            strcpy(name, lexemes[current_token].lexeme);
            current_token++;
            if (lexemes[current_token].token != eqlsym) {
                error("constants must be assigned with =", NULL);
            }
            current_token++;
            if (lexemes[current_token].token != numbersym) {
                error("constants must be assigned an integer value", NULL);
            }
            int value = atoi(lexemes[current_token].lexeme);
            add_symbol(1, name, value, current_level, 0);
            current_token++;
        } while (lexemes[current_token].token == commasym);
        if (lexemes[current_token].token != semicolonsym) {
            error("constant declarations must be followed by a semicolon", NULL);
        }
        current_token++;
    }
}

// var_declaration is a helper function for block that handles the declaration of variables
int var_declaration() {
    int numVars = 0;
    if (lexemes[current_token].token == varsym) {
        do {
            current_token++;
            if (lexemes[current_token].token != identsym) {
                error("var must be followed by identifier", NULL);
            }
            if (symbol_table_check(lexemes[current_token].lexeme) != -1) {
                error("symbol name has already been declared", lexemes[current_token].lexeme);
            }
            add_symbol(2, lexemes[current_token].lexeme, 0, current_level, 3 + numVars);
            numVars++;
            current_token++;
        } while (lexemes[current_token].token == commasym);
        if (lexemes[current_token].token != semicolonsym) {
            error("variable declarations must be followed by a semicolon", NULL);
        }
        current_token++;
    }
    return numVars;
}

// procedure_declaration is a helper function for block that handles the declaration of procedures
void procedure_declaration(int level, int *tx, int *dx) {
    while (lexemes[current_token].token == procsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error("procedure must be followed by identifier", NULL);
        }
        char procName[MAX_IDENT_LEN + 1];
        strcpy(procName, lexemes[current_token].lexeme);
        current_token++;
        if (lexemes[current_token].token != semicolonsym) {
            error("semicolon expected after procedure declaration", NULL);
        }
        current_token++;
        
        int procIndex = code_index;
        add_symbol(3, procName, 0, level, procIndex);
        current_level++;
        block(level + 1, *tx);
        if (lexemes[current_token].token != semicolonsym) {
            error("semicolon expected after procedure block", NULL);
        }
        current_token++;
        emit("RTN", 0, 0);
        current_level--;
    }
}
void statement() {
    printf("Debug: Entering statement with token: %d (%s)\n", lexemes[current_token].token, lexemes[current_token].lexeme);

    if (lexemes[current_token].token == identsym) {
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        printf("Debug: Found identifier %s with symIdx: %d\n", lexemes[current_token].lexeme, symIdx);

        if (symIdx == -1) {
            error("undeclared identifier", lexemes[current_token].lexeme);
        }

        // Check if the symbol is a constant or a procedure
        if (symbol_table[symIdx].kind == 1) {
            error("assignment to constant is not allowed", lexemes[current_token].lexeme);
        }
        if (symbol_table[symIdx].kind == 3) {
            error("assignment to procedure is not allowed", lexemes[current_token].lexeme);
        }

        current_token++;
        if (lexemes[current_token].token != becomessym) {
            error("assignment statements must use :=", NULL);
        }

        current_token++;
        expression();
        emit("STO", current_level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
    } else if (lexemes[current_token].token == beginsym) {
        printf("Debug: Processing begin block\n");
        current_token++;
        statement();
        while (lexemes[current_token].token == semicolonsym) {
            current_token++;
            statement();
        }
        if (lexemes[current_token].token != endsym) {
            error("begin must be followed by end", NULL);
        }
        current_token++;
    } else if (lexemes[current_token].token == ifsym) {
        current_token++;
        condition();
        int jpcIdx = code_index;
        emit("JPC", 0, 0);
        if (lexemes[current_token].token != thensym) {
            error("if must be followed by then", NULL);
        }
        current_token++;
        statement();
        code[jpcIdx].m = code_index;
    } else if (lexemes[current_token].token == whilesym) {
        current_token++;
        int loopIdx = code_index;
        condition();
        if (lexemes[current_token].token != dosym) {
            error("while must be followed by do", NULL);
        }
        current_token++;
        int jpcIdx = code_index;
        emit("JPC", 0, 0);
        statement();
        emit("JMP", 0, loopIdx);
        code[jpcIdx].m = code_index;
    } else if (lexemes[current_token].token == readsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error("read must be followed by identifier", NULL);
        }
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx == -1) {
            error("undeclared identifier", lexemes[current_token].lexeme);
        }
        if (symbol_table[symIdx].kind != 2) {
            error("only variable values may be altered", NULL);
        }
        current_token++;
        emit("SYS", 0, 2); // Assuming SYS 2 is read
        emit("STO", current_level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
    } else if (lexemes[current_token].token == writesym) {
        current_token++;
        expression();
        emit("SYS", 0, 1); // Assuming SYS 1 is write
    } else if (lexemes[current_token].token == callsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error("call must be followed by an identifier", NULL);
        }
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        printf("Debug: Processing call with symIdx: %d\n", symIdx);
        if (symIdx == -1 || symbol_table[symIdx].kind != 3) {
            error("call must be followed by a procedure identifier", lexemes[current_token].lexeme);
        }
        current_token++;
        emit("CAL", current_level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
    } else {
        printf("Debug: Invalid statement with token: %d (%s)\n", lexemes[current_token].token, lexemes[current_token].lexeme);
        error("Invalid statement", lexemes[current_token].lexeme);
    }
}




// condition is a helper function for statement that handles the different types of conditions that can be declared in a statement
void condition() {
    if (lexemes[current_token].token == oddsym) {
        current_token++;
        expression();
        emit("ODD", 0, 0);
    } else {
        expression();
        if (lexemes[current_token].token != eqlsym && lexemes[current_token].token != neqsym &&
            lexemes[current_token].token != lessym && lexemes[current_token].token != leqsym &&
            lexemes[current_token].token != gtrsym && lexemes[current_token].token != geqsym) {
            error("condition must contain comparison operator", NULL);
        }
        token_type relOp = lexemes[current_token].token;
        current_token++;
        expression();
        switch (relOp) {
            case eqlsym: emit("EQL", 0, 0); break;
            case neqsym: emit("NEQ", 0, 0); break;
            case lessym: emit("LSS", 0, 0); break;
            case leqsym: emit("LEQ", 0, 0); break;
            case gtrsym: emit("GTR", 0, 0); break;
            case geqsym: emit("GEQ", 0, 0); break;
            default:
                error("condition must contain comparison operator", NULL);
        }
    }
}

// expression is a helper function for condition that handles the different types of expressions that can be declared in a condition
void expression() {
    if (lexemes[current_token].token == identsym) {
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx != -1 && symbol_table[symIdx].kind == 3) {
            error("Expression cannot contain a procedure identifier", lexemes[current_token].lexeme);
        }
    }

    if (lexemes[current_token].token == plussym || lexemes[current_token].token == minussym) {
        token_type addOp = lexemes[current_token].token;
        current_token++;
        term();
        if (addOp == minussym) {
            emit("NEG", 0, 0);
        }
    } else {
        term();
    }
    while (lexemes[current_token].token == plussym || lexemes[current_token].token == minussym) {
        token_type addOp = lexemes[current_token].token;
        current_token++;
        term();
        if (addOp == plussym) {
            emit("ADD", 0, 0);
        } else {
            emit("SUB", 0, 0);
        }
    }
}

// term is a helper function for expression that handles the different types of terms that can be declared in an expression
void term() {
    factor();
    while (lexemes[current_token].token == multsym || lexemes[current_token].token == slashsym) {
        token_type mulOp = lexemes[current_token].token;
        current_token++;
        factor();
        if (mulOp == multsym) {
            emit("MUL", 0, 0); // Assuming MUL is OPR 0 2, if different, adjust accordingly
        } else {
            emit("DIV", 0, 0); // Assuming DIV is OPR 0 4, if different, adjust accordingly
        }
    }
}

// factor is a helper function for term that handles the different types of factors that can be declared in a term
void factor() {
    if (lexemes[current_token].token == identsym) {
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx == -1) {
            error("undeclared identifier", lexemes[current_token].lexeme);
        }
        if (symbol_table[symIdx].kind == 1) {
            emit("LIT", 0, symbol_table[symIdx].val);
        } else {
            emit("LOD", current_level - symbol_table[symIdx].level, symbol_table[symIdx].addr);
        }
        current_token++;
    } else if (lexemes[current_token].token == numbersym) {
        emit("LIT", 0, atoi(lexemes[current_token].lexeme));
        current_token++;
    } else if (lexemes[current_token].token == lparentsym) {
        current_token++;
        expression();
        if (lexemes[current_token].token != rparentsym) {
            error("right parenthesis must follow left parenthesis", NULL);
        }
        current_token++;
    } else {
        error("arithmetic equations must contain operands, parentheses, numbers, or symbols", NULL);
    }
}
