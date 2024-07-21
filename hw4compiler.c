/*
July 7, 2024
COP 3402 Systems Software Assignment 4
This program is written by: Devon Villalona and Izaac Plambeck
HW4: PL/0 Compiler
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

// token_type enum
typedef enum {
    oddsym = 1, identsym, numbersym, plussym, minussym,
    multsym, slashsym, eqlsym, neqsym, lessym, leqsym,
    gtrsym, geqsym, lparentsym, rparentsym, commasym, semicolonsym,
    periodsym, becomessym, beginsym, endsym, ifsym, thensym, whilesym,
    dosym, callsym, constsym, varsym, procsym, readsym, writesym
} token_type;

typedef struct {
    char lexeme[MAX_IDENT_LEN + 1];
    token_type token;
    char error[50];
} Lexeme;

typedef struct {
    int kind;
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

char *reservedWords[] = {
    "const", "var", "procedure", "call", "begin", "end", "if",
    "then", "while", "do", "read", "write"
};

// Reserved words
token_type reservedTokens[] = {
    constsym, varsym, procsym, callsym, beginsym, endsym, ifsym,
    thensym, whilesym, dosym, readsym, writesym
};

// Function prototypes
void addLexeme(char *lex, token_type token, char *error);
token_type identifyReservedWord(char *word);
void processInput(const char *input);
void readInputFile(const char *filename, char *buffer, int size);
void writeOutputFile(const char *filename);
void writeCode();
void error(int errorNum, const char *message);
void program();
void block();
void const_declaration();
int var_declaration();
void procedure_declaration();
void statement();
void condition();
void expression();
void term();
void factor();
int symbol_table_check(const char *name);
void add_symbol(int kind, const char *name, int val, int level, int addr);
void emit(const char *op, int l, int m);
void mark_all_symbols();

// Function implementations

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

token_type identifyReservedWord(char *word) {
    for (int i = 0; i < sizeof(reservedWords) / sizeof(reservedWords[0]); i++) {
        if (strcmp(word, reservedWords[i]) == 0) {
            return reservedTokens[i];
        }
    }
    return identsym;
}

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
                addLexeme(buffer, identsym, "Name too long");
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
                addLexeme(buffer, numbersym, "Number too long");
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
                            addLexeme("/*", identsym, "Unterminated comment");
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
                        addLexeme(buffer, identsym, "Invalid character");
                    }
                    break;
                case ';': addLexeme(buffer, semicolonsym, NULL); break;
                case ',': addLexeme(buffer, commasym, NULL); break;
                case '.': addLexeme(buffer, periodsym, NULL); break;
                case '(': addLexeme(buffer, lparentsym, NULL); break;
                case ')': addLexeme(buffer, rparentsym, NULL); break;
                default:
                    addLexeme(buffer, identsym, "Invalid character");
                    break;
            }
        }
    }
}

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

    fprintf(file, "Symbol Table:\n");
    fprintf(file, "Kind | Name | Value | Level | Address | Mark\n");
    fprintf(file, "---------------------------------------------------\n");
    for (int i = 0; i < symbol_count; i++) {
        fprintf(file, "%d | %s | %d | %d | %d | %d\n", symbol_table[i].kind,
                symbol_table[i].name, symbol_table[i].val, symbol_table[i].level,
                symbol_table[i].addr, symbol_table[i].mark);
    }

    fclose(file);
}

void error(int errorNum, const char *message) {
    printf("Error: Error number %d, %s\n", errorNum, message);
    exit(EXIT_FAILURE);
}

int symbol_table_check(const char *name) {
    for (int i = 0; i < symbol_count; i++) {
        if (strcmp(symbol_table[i].name, name) == 0 && symbol_table[i].mark == 0) {
            return i;
        }
    }
    return -1;
}

void add_symbol(int kind, const char *name, int val, int level, int addr) {
    if (symbol_count >= MAX_SYMBOL_TABLE_SIZE) {
        error(5, "Symbol table overflow");
    }
    symbol_table[symbol_count].kind = kind;
    strncpy(symbol_table[symbol_count].name, name, MAX_IDENT_LEN);
    symbol_table[symbol_count].val = val;
    symbol_table[symbol_count].level = level;
    symbol_table[symbol_count].addr = addr;
    symbol_table[symbol_count].mark = 0;
    symbol_count++;
}

void emit(const char *op, int l, int m) {
    if (code_index >= MAX_CODE_LENGTH) {
        error(6, "Code size exceeded");
    }
    strncpy(code[code_index].op, op, 4);
    code[code_index].l = l;
    code[code_index].m = m;
    code_index++;
}

void mark_all_symbols() {
    for (int i = 0; i < symbol_count; i++) {
        symbol_table[i].mark = 1;
    }
}

void program() {
    emit("JMP", 0, 3);
    block();
    if (lexemes[current_token].token != periodsym) {
        error(9, "Period expected");
    }
    emit("SYS", 0, 3);
    mark_all_symbols();
}

void block() {
    const_declaration();
    int numVars = var_declaration();
    procedure_declaration();
    emit("INC", 0, 3 + numVars);
    statement();
}

void const_declaration() {
    if (lexemes[current_token].token == constsym) {
        do {
            current_token++;
            if (lexemes[current_token].token != identsym) {
                error(4, "const must be followed by identifier");
            }
            if (symbol_table_check(lexemes[current_token].lexeme) != -1) {
                error(4, "symbol name has already been declared");
            }
            char name[MAX_IDENT_LEN + 1];
            strcpy(name, lexemes[current_token].lexeme);
            current_token++;
            if (lexemes[current_token].token != eqlsym) {
                error(2, "constants must be assigned with =");
            }
            current_token++;
            if (lexemes[current_token].token != numbersym) {
                error(2, "constants must be assigned an integer value");
            }
            int value = atoi(lexemes[current_token].lexeme);
            add_symbol(1, name, value, 0, 0);
            current_token++;
        } while (lexemes[current_token].token == commasym);
        if (lexemes[current_token].token != semicolonsym) {
            error(5, "constant declarations must be followed by a semicolon");
        }
        current_token++;
    }
}

int var_declaration() {
    int numVars = 0;
    if (lexemes[current_token].token == varsym) {
        do {
            current_token++;
            if (lexemes[current_token].token != identsym) {
                error(4, "var must be followed by identifier");
            }
            if (symbol_table_check(lexemes[current_token].lexeme) != -1) {
                error(4, "symbol name has already been declared");
            }
            add_symbol(2, lexemes[current_token].lexeme, 0, 0, 3 + numVars);
            numVars++;
            current_token++;
        } while (lexemes[current_token].token == commasym);
        if (lexemes[current_token].token != semicolonsym) {
            error(5, "variable declarations must be followed by a semicolon");
        }
        current_token++;
    }
    return numVars;
}

void procedure_declaration() {
    while (lexemes[current_token].token == procsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error(4, "procedure must be followed by identifier");
        }
        char name[MAX_IDENT_LEN + 1];
        strcpy(name, lexemes[current_token].lexeme);
        add_symbol(3, name, 0, 0, code_index + 1);
        current_token++;
        if (lexemes[current_token].token != semicolonsym) {
            error(5, "Semicolon or comma missing");
        }
        current_token++;
        block();
        if (lexemes[current_token].token != semicolonsym) {
            error(5, "Semicolon or comma missing");
        }
        current_token++;
    }
}

void statement() {
    if (lexemes[current_token].token == identsym) {
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx == -1) {
            error(11, "undeclared identifier");
        }
        if (symbol_table[symIdx].kind != 2) {
            error(12, "Assignment to constant or procedure is not allowed");
        }
        current_token++;
        if (lexemes[current_token].token != becomessym) {
            if (lexemes[current_token].token == eqlsym) {
                error(1, "Use = instead of :=");
            }
            error(13, "assignment statements must use :=");
        }
        current_token++;
        expression();
        emit("STO", 0, symbol_table[symIdx].addr);
    } else if (lexemes[current_token].token == beginsym) {
        current_token++;
        do {
            statement();
            if (lexemes[current_token].token == semicolonsym) {
                current_token++;
            }
        } while (lexemes[current_token].token == semicolonsym);
        if (lexemes[current_token].token != endsym) {
            error(17, "begin must be followed by end");
        }
        current_token++;
    } else if (lexemes[current_token].token == ifsym) {
        current_token++;
        condition();
        if (lexemes[current_token].token != thensym) {
            error(16, "if must be followed by then");
        }
        current_token++;
        statement();
        if (lexemes[current_token].token != endsym) {
            error(17, "Semicolon or end expected");
        }
        current_token++;
    } else if (lexemes[current_token].token == whilesym) {
        current_token++;
        int loopIdx = code_index;
        condition();
        if (lexemes[current_token].token != dosym) {
            error(18, "while must be followed by do");
        }
        current_token++;
        int jpcIdx = code_index;
        emit("JPC", 0, 0);
        statement();
        emit("JMP", 0, loopIdx);
        code[jpcIdx].m = code_index;
    } else if (lexemes[current_token].token == callsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error(14, "call must be followed by an identifier");
        }
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx == -1) {
            error(11, "undeclared identifier");
        }
        if (symbol_table[symIdx].kind != 3) {
            error(15, "Call of a constant or variable is meaningless");
        }
        emit("CAL", 0, symbol_table[symIdx].addr);
        current_token++;
    } else if (lexemes[current_token].token == readsym) {
        current_token++;
        if (lexemes[current_token].token != identsym) {
            error(4, "read must be followed by identifier");
        }
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx == -1) {
            error(11, "undeclared identifier");
        }
        if (symbol_table[symIdx].kind != 2) {
            error(12, "Assignment to constant or procedure is not allowed");
        }
        current_token++;
        emit("READ", 0, 0);
        emit("STO", 0, symbol_table[symIdx].addr);
    } else if (lexemes[current_token].token == writesym) {
        current_token++;
        expression();
        emit("WRITE", 0, 0);
    } else {
        error(7, "Invalid statement");
    }
}

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
            error(20, "condition must contain comparison operator");
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
                error(20, "condition must contain comparison operator");
        }
    }
}

void expression() {
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

void term() {
    factor();
    while (lexemes[current_token].token == multsym || lexemes[current_token].token == slashsym) {
        token_type mulOp = lexemes[current_token].token;
        current_token++;
        factor();
        if (mulOp == multsym) {
            emit("MUL", 0, 0); // Emit the correct instruction for multiplication
        } else {
            emit("DIV", 0, 0); // Assuming DIV is OPR 0 4, if different, adjust accordingly
        }
    }
}

void factor() {
    if (lexemes[current_token].token == identsym) {
        int symIdx = symbol_table_check(lexemes[current_token].lexeme);
        if (symIdx == -1) {
            error(11, "undeclared identifier");
        }
        if (symbol_table[symIdx].kind == 1) {
            emit("LIT", 0, symbol_table[symIdx].val);
        } else {
            emit("LOD", 0, symbol_table[symIdx].addr);
        }
        current_token++;
    } else if (lexemes[current_token].token == numbersym) {
        emit("LIT", 0, atoi(lexemes[current_token].lexeme));
        current_token++;
    } else if (lexemes[current_token].token == lparentsym) {
        current_token++;
        expression();
        if (lexemes[current_token].token != rparentsym) {
            error(22, "right parenthesis must follow left parenthesis");
        }
        current_token++;
    } else {
        error(24, "arithmetic equations must contain operands, parentheses, numbers, or symbols");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const int bufferSize = 10000;
    char input[bufferSize];

    readInputFile(argv[1], input, bufferSize);
    processInput(input);

    printf("Source Program:\n%s\n", input);

    program();

    printf("No errors, program is syntactically correct\n");

    writeCode();
    writeOutputFile("output.txt");

    return 0;
}
