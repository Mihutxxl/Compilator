#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"

#define MAX_TOKEN_LEN 256
#define SAFEALLOC(var,Type) if((var=(Type*)malloc(sizeof(Type)))==NULL) { \
    fprintf(stderr, "not enough memory\n"); \
    exit(1); \
}
#define INITIAL_CAPACITY 10

//Enum for the different token types - kept in lexer.h

//Struct to represent a token - kept in lexer.h

//List of the reserved keywords
const char *keywords[] = {"if", "else", "while", "return", "int", "float", "char", "void", "for", "double", NULL};

//Function to check if a string is a keyword
int is_keyword(const char *str) {
    for(int i = 0; keywords[i] != NULL; i++) {
        if(strcmp(str,keywords[i]) == 0) return 1;  //Return 1 if keyword
    }
    return 0;   //Return 0 if not keyword
}

// Helper function to check if a character is a hexadecimal digit
int is_hex_digit(char c) {
    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

// Helper function to check if a character is an octal digit
int is_octal_digit(char c) {
    return c >= '0' && c <= '7';
}

// Helper function to handle real number tokenization
void handle_real_number(const char **input, Token *token) {
    int i = 0;
    int has_exp = 0;
    
    // Integer part
    while(isdigit(**input)) {
        token->value[i++] = *(*input)++;
    }
    
    // Decimal point and fraction
    if(**input == '.') {
        token->value[i++] = *(*input)++;
        while(isdigit(**input)) {
            token->value[i++] = *(*input)++;
        }
    }
    
    // Exponent part
    if(**input == 'e' || **input == 'E') {
        has_exp = 1;
        token->value[i++] = *(*input)++;
        if(**input == '+' || **input == '-') {
            token->value[i++] = *(*input)++;
        }
        if(isdigit(**input)) {
            while(isdigit(**input)) {
                token->value[i++] = *(*input)++;
            }
        } else {
            token->type = TOKEN_ERROR;
            token->value[i] = '\0';
            return;
        }
    }
    
    token->value[i] = '\0';
    
    // Valid real number must have either a decimal part or an exponent
    if(has_exp || (i > 0 && token->value[0] == '.') || (strchr(token->value, '.') != NULL)) {
        token->type = TOKEN_REAL;
    } else {
        token->type = TOKEN_ERROR;
    }
}

//Function to retrieve the next token from the input string
Token get_token(const char **input) {
    Token token;    //Initialize token struct
    token.value[0] = '\0';  //Initialize token value to an empty string

    //Skip all the whitespaces
    while(isspace(**input)){
        (*input)++; //Move to the next character
    }

    //If we reach the end of the string, return an EOF token
    if(**input == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }

    //Handle identifiers (Starts with a letter or an underscore)
    if(isalpha(**input) || **input == '_') {
        int i = 0;
        // Continue adding characters to token value as long as they are alphanumeric or underscore
        while (isalnum(**input) || **input == '_') {
            token.value[i++] = *(*input)++;
        }
        token.value[i] = '\0';  // Null-terminate the string
        token.type = is_keyword(token.value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;    // Check if it's a keyword
    } else if(isdigit(**input)) {
        // Handle numbers (digits only)
        int i = 0;
        
        // Hexadecimal: starts with '0x'
        if(**input == '0' && (*(*input + 1) == 'x' || *(*input + 1) == 'X')) {
            token.value[i++] = *(*input)++; // '0'
            token.value[i++] = *(*input)++; //'x' or 'X'
            if(is_hex_digit(**input)) {
                while(is_hex_digit(**input)) {
                    token.value[i++] = *(*input)++;
                }
                token.type = TOKEN_NUMBER_HEX;
            } else {
                token.type = TOKEN_ERROR;
            }
        } else if(**input == '0') { // Octal: starts with '0'
            token.value[i++] = *(*input)++; // '0'
            while(is_octal_digit(**input)) {
                token.value[i++] = *(*input)++;
            }
            token.type = TOKEN_NUMBER_OCT;
        } else if(**input >= '1' && **input <='9') {
            // Check if this might be a real number
            if((*(*input + 1) == '.' && isdigit(*(*input + 2))) || 
               (isdigit(*(*input + 1)) && (*(*input + 2) == '.' || *(*input + 2) == 'e' || *(*input + 2) == 'E'))) {
                handle_real_number(input, &token);
                return token;
            }
            
            while(isdigit(**input)) {
                token.value[i++] = *(*input)++;
            }
            token.type = TOKEN_NUMBER_ZEC;
        }
        token.value[i] = '\0';
    } else if(**input == '.' && isdigit(*(*input + 1))) {   // Handle the real numbers
        handle_real_number(input, &token);
    } else if(**input == '"') {
        // Handle strings (delimited by double quotes)
        int i = 0;
        (*input)++; // Skip the opening quote
        // Continue adding characters to token value until closing quote is found
        while(**input && **input != '"') {
            token.value[i++] = *(*input)++;
        }
        if(**input == '"')
            (*input)++; // Skip the closing quote
        token.value[i] = '\0';  // Null-terminate the string
        token.type = TOKEN_STRING;  // Token is a string
    } else if(**input == '\'') {
        int i = 0;
        token.value[i++] = *(*input)++; //Store the opening quote
        //Handle escaped characters
        if(**input == '\\') {
            token.value[i++] = *(*input)++; //Store the backslash
            if(**input) {
                token.value[i++] = *(*input)++; //Store escaped characters
            }
        } else if(**input && **input != '\'') {
            token.value[i++] = *(*input)++; //Store the character
        }
        //Check for closing quote
        if(**input =='\'') {
            token.value[i++] = *(*input)++; //Store the closing quote
        }
        token.value[i] = '\0';   //Null-terminate the string
        token.type = TOKEN_CHAR_LITERAL;
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
                (*input)++; // Skip characters in the comment
            }
            token.value[i] = '\0';  // Null-terminate the string
            token.type = TOKEN_LINECOMMENT; // Token is a line comment
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
                    found_end = 1;  // End of comment found
                } else {
                    if(i < MAX_TOKEN_LEN - 3) {  // Leave space for */ and null terminator
                        token.value[i++] = *(*input);
                    }
                    (*input)++; // Continue to the next character
                }
                
                if(**input == '\0') {
                    break;  // Break if we reach the end of the input
                }
            }
            token.value[i] = '\0';  // Null-terminate the string
            token.type = TOKEN_MULTILINECOMMENT;    // Token is a multi-line comment
        }
        else {
            // Just a division operator
            token.type = TOKEN_DIVIDE;
            token.value[0] = *(*input)++;   // Store the '/' character
            token.value[1] = '\0';  // Null-terminate the string
        }
    } else if(**input == '=') {
        if(*(*input + 1) == '=') {    // Equal operator (==)
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_EQUAL;
        } else {    // Assignment operator (=)
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_ASSIGN;
        }
    } else if(**input == '<') {
        if(*(*input + 1) == '=') {    // Less than or equal operator (<=)
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_LESSEQUAL;
        } else {    // Less than operator (<)
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_LESS;
        }
    } else if(**input == '>') {
        if(*(*input + 1) == '=') {    // Greater than or equal operator (>=)
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_GREATEREQUAL;
        } else {    // Greater than operator (>)
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_GREATER;
        }
    } else if(**input == '!') {
        if(*(*input + 1) == '=') {    // Not equal operator (!=)
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_NOTEQUAL;
        } else {    // Not operator (!)
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_NOT;
        }
    } else if(**input == '&') {
        if(*(*input + 1) == '&') {  // And operator (&&)
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_AND;
        } else {    // No bitwise and for atomC
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_ERROR;
        }
    } else if(**input == '|') {
        if(*(*input + 1) == '|') {  // Or operator (||)
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_OR;
        } else {    // No bitwise or for atomC
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_ERROR;
        }
    } else if(**input == '+') {
        if(*(*input + 1) == '+') {  // ++ operator
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_PLUS_1;
        } else {    // + operator
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_PLUS;
        }
    } else if(**input == '-') {
        if(*(*input + 1) == '-') {  // -- operator
            token.value[0] = *(*input)++;
            token.value[1] = *(*input)++;
            token.value[2] = '\0';  // Null-terminate the string
            token.type = TOKEN_MINUS_1;  // Fixed: was TOKEN_PLUS_1
        } else {    // - operator
            token.value[0] = *(*input)++;
            token.value[1] = '\0';  // Null-terminate the string
            token.type = TOKEN_MINUS;  // Fixed: was TOKEN_PLUS
        }
    }
    // Handle other single-character tokens (operators, parentheses, etc.)
    else {
        switch (**input) {
            case '*': token.type = TOKEN_MULTIPLY; break;
            case ';': token.type = TOKEN_SEMICOLON; break;
            case '(': token.type = TOKEN_LPAREN; break;
            case ')': token.type = TOKEN_RPAREN; break;
            case '[': token.type = TOKEN_LBRACKET; break;
            case ']': token.type = TOKEN_RBRACKET; break;
            case '{': token.type = TOKEN_LBRACE; break;
            case '}': token.type = TOKEN_RBRACE; break;
            case ',': token.type = TOKEN_COMMA; break;
            case '.': token.type = TOKEN_DOT; break; 
            default: token.type = TOKEN_ERROR; break;   // Unknown character
        }
        token.value[0] = **input;   // Store the character
        token.value[1] = '\0';  // Null-terminate the string
        (*input)++; // Move to the next character
    }
    return token;   // Return the constructed token
}

// Function to read the entire content of a file into a string
char *read_file(const char *filename){
    FILE *file = fopen(filename, "rb");  // Open the file in read mode
    
    if(!file) {
        fprintf(stderr, "Error opening file: %s\n", filename);
        exit(1);
    }
    
    // Find the file length
    fseek(file, 0, SEEK_END); 
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // Allocate memory for the file content
    char *buffer = (char *)malloc(length + 1);
    if(!buffer){
        fprintf(stderr, "Memory allocation failure\n");
        fclose(file);
        exit(1);
    }
    
    // Read file content into buffer
    size_t bytes_read = fread(buffer, 1, length, file);
    if (bytes_read < (size_t)length) {
        fprintf(stderr, "Warning: Only read %zu of %ld bytes\n", bytes_read, length);
    }
    
    buffer[bytes_read] = '\0';  // Null-terminate the string
    fclose(file);   // Close the file
    return buffer;  // Return the file content
}

// Function to handle errors
void err(const char *message) {
    fprintf(stderr, "%s\n", message);
    exit(1);
}

// Main function to process the input file
Token *tokenize_file(const char *filename, int *token_count) {
    char *source = read_file(filename);
    const char *input = source;
    
    int capacity = INITIAL_CAPACITY;
    *token_count = 0;
    Token *tokens = (Token *)malloc(capacity * sizeof(Token));
    if (!tokens) {
        fprintf(stderr, "Memory allocation failed for tokens!\n");
        free(source);
        return NULL;
    }

    Token token;
    while ((token = get_token(&input)).type != TOKEN_EOF) {
        if (*token_count >= capacity) {
            capacity *= 2;
            Token *new_tokens = (Token *)realloc(tokens, capacity * sizeof(Token));
            if (!new_tokens) {
                fprintf(stderr, "Memory reallocation failed!\n");
                free(tokens);
                free(source);
                return NULL;
            }
            tokens = new_tokens;
        }
        tokens[(*token_count)++] = token;
    }

    free(source);
    return tokens;  // Return dynamic array
}