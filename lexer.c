#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKEN_LEN 100
#define SAFEALLOC(var,Type)if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

typedef enum {
    TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING, TOKEN_PLUS, TOKEN_MINUS,
    TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_ASSIGN, TOKEN_SEMICOLON,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_COMMA, TOKEN_KEYWORD, TOKEN_COMMENT, TOKEN_ERROR, TOKEN_EOF,
    TOKEN_LESS, TOKEN_GREATER, TOKEN_LESSEQUAL, TOKEN_GREATEREQUAL,
    TOKEN_EQUAL, TOKEN_NOTEQUAL, TOKEN_LINECOMMENT, TOKEN_MULTYLINECOMMENT
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
}Token;

const char *keywords[] = {"if", "else", "while", "return", "int", "float", "char", "void", NULL};

int is_keyword(const char *str) {
    for(int i = 0; keywords[i] != NULL; i++) {
        if(strcmp(str,keywords[i]) == 0) return 1;
    }
    return 0;
}

Token get_token(const char **input) {
    Token token;
    token.value[0] = '\0';

    while(isspace(**input)){
        (*input)++;
    }

    if(**input == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }

    if(isalpha(**input) || **input == '_') {
        int i = 0;
        while (isalnum(**input) || **input == '_') {
            token.value[i++] = *(*input)++;
        }
        token.value[i] = '\0';
        token.type = is_keyword(token.value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    } else if(isdigit(**input)) {
        int i = 0;
        while(isdigit(**input)) {
            token.value[i++] = *(*input)++;
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
    } else if(**input == '"') {
        int i = 0;
        (*input)++;
        while(**input && **input != '"') {
            token.value[i++] = *(*input)++;
        }
        if(**input == '"')
            (*input)++;
        token.value[i] = '\0';
        token.type = TOKEN_STRING;
    } else if(**input == '/') {
        // Handle comments or division operator
        if(*(*input + 1) == '/') {
            // Line comment
            int i = 0;
            token.value[i++] = *(*input)++;  // Store first '/'
            token.value[i++] = *(*input)++;  // Store second '/'
            while (**input && **input != '\n') {
                if (i < MAX_TOKEN_LEN - 1) {
                    token.value[i++] = *(*input);
                }
                (*input)++;
            }
            token.value[i] = '\0';
            token.type = TOKEN_LINECOMMENT;
        }
        else if(*(*input + 1) == '*') {
            // Multi-line comment
            int i = 0;
            token.value[i++] = *(*input)++;  // Store '/'
            token.value[i++] = *(*input)++;  // Store '*'
            // Flag to track if we've found the end of the comment
            int found_end = 0;
            while(**input && !found_end) {
                if(**input == '*' && *(*input + 1) == '/') {
                    if(i < MAX_TOKEN_LEN - 2) {
                        token.value[i++] = *(*input)++;  // Store '*'
                        token.value[i++] = *(*input)++;  // Store '/'
                    } else {
                        (*input) += 2;  // Skip anyway if buffer full
                    }
                    found_end = 1;
                } else {
                    if(i < MAX_TOKEN_LEN - 3) {  // Leave space for */ and null terminator
                        token.value[i++] = *(*input);
                    }
                    (*input)++;
                }
                
                if(**input == '\0') {
                    break;
                }
            }
            token.value[i] = '\0';
            token.type = TOKEN_MULTYLINECOMMENT;
        }
        else {
            // Just a division operator
            token.type = TOKEN_DIVIDE;
            token.value[0] = *(*input)++;
            token.value[1] = '\0';
        }
    }
    
    else {
        switch (**input) {
            case '+': token.type = TOKEN_PLUS; break;
            case '-': token.type = TOKEN_MINUS; break;
            case '*': token.type = TOKEN_MULTIPLY; break;
            case '=': token.type = (*(*input + 1) == '=') ? (*input += 1, TOKEN_EQUAL) : TOKEN_ASSIGN; break;
            case '<': token.type = (*(*input + 1) == '=') ? (*input += 1, TOKEN_LESSEQUAL) : TOKEN_LESS; break;
            case '>': token.type = (*(*input + 1) == '=') ? (*input += 1, TOKEN_GREATEREQUAL) : TOKEN_GREATER; break;
            case '!': token.type = (*(*input + 1) == '=') ? (*input += 1, TOKEN_NOTEQUAL) : TOKEN_ERROR; break;
            case ';': token.type = TOKEN_SEMICOLON; break;
            case '(': token.type = TOKEN_LPAREN; break;
            case ')': token.type = TOKEN_RPAREN; break;
            case '{': token.type = TOKEN_LBRACE; break;
            case '}': token.type = TOKEN_RBRACE; break;
            case ',': token.type = TOKEN_COMMA; break;
            default: token.type = TOKEN_ERROR; break;
        }
        token.value[0] = **input;
        token.value[1] = '\0';
        (*input)++;
    }
    return token;
}

char *read_file(const char *filename){
    FILE *file = fopen(filename, "r");
    
    if(!file) {
        perror("Error opening file\n");
        exit(-1);
    }

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0,SEEK_SET);
    char *buffer = malloc(length + 1);
    if(!buffer){
        perror("Memory allocation failure\n");
        exit(-1);
    }
    fread(buffer, 1, length, file);
    buffer[length] = '\0';
    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    char *source = read_file(argv[1]);
    const char *input = source;
    Token token;
    while((token = get_token(&input)).type != TOKEN_EOF) {
        printf("Token: Type=%d, Value='%s'\n", token.type, token.value);
    }

    free(source);
    return 0;
}
