#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKEN_LEN 256
#define SAFEALLOC(var,Type)if((var=(Type*)malloc(sizeof(Type)))==NULL)err("not enough memory");

//Enum for the different token types
typedef enum {
    TOKEN_IDENTIFIER, TOKEN_NUMBER, TOKEN_STRING, TOKEN_PLUS, TOKEN_MINUS,
    TOKEN_MULTIPLY, TOKEN_DIVIDE, TOKEN_ASSIGN, TOKEN_SEMICOLON,
    TOKEN_LPAREN, TOKEN_RPAREN, TOKEN_LBRACE, TOKEN_RBRACE,
    TOKEN_COMMA, TOKEN_KEYWORD, TOKEN_COMMENT, TOKEN_ERROR, TOKEN_EOF,
    TOKEN_LESS, TOKEN_GREATER, TOKEN_LESSEQUAL, TOKEN_GREATEREQUAL,
    TOKEN_EQUAL, TOKEN_NOTEQUAL, TOKEN_LINECOMMENT, TOKEN_MULTYLINECOMMENT,
    TOKEN_CHAR_LITERAL
} TokenType;

//Struct to represent a token
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LEN];
}Token;

//List of the reserved keywords
const char *keywords[] = {"if", "else", "while", "return", "int", "float", "char", "void", NULL};

//Function to check if a string is a keyword
int is_keyword(const char *str) {
    for(int i = 0; keywords[i] != NULL; i++) {
        if(strcmp(str,keywords[i]) == 0) return 1;  //Return 1 if keyword
    }
    return 0;   //Return 0 if not keyword
}

//Function to retrieve the next token from the input string
Token get_token(const char **input) {
    Token token;    //Initialize token struct
    token.value[0] = '\0';  //Initialize token value to an empty string

    //Skip all the whitespaces
    while(isspace(**input)){
        (*input)++; //Move to the next charecter
    }

    //If we reach the end of the string, return an EOF token
    if(**input == '\0') {
        token.type = TOKEN_EOF;
        return token;
    }

    //Handel identifiers (Starts with a letter or an underscore)
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
        // Continue adding digits to token value
        while(isdigit(**input)) {
            token.value[i++] = *(*input)++;
        }
        token.value[i] = '\0';  // Null-terminate the string
        token.type = TOKEN_NUMBER;  // Token is a number
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
            token.type = TOKEN_MULTYLINECOMMENT;    // Token is a multi-line comment
        }
        else {
            // Just a division operator
            token.type = TOKEN_DIVIDE;
            token.value[0] = *(*input)++;   // Store the '/' character
            token.value[1] = '\0';  // Null-terminate the string
        }
    }
    // Handle other single-character tokens (operators, parentheses, etc.)
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
    FILE *file = fopen(filename, "r");  // Open the file in read mode
    
    if(!file) {
        perror("Error opening file\n"); // Print error if file cannot be opened
        exit(-1);
    }
    // Find the file length
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0,SEEK_SET);
    // Allocate memory for the file content
    char *buffer = malloc(length + 1);
    if(!buffer){
        perror("Memory allocation failure\n");  // Print error if memory allocation fails
        exit(-1);
    }
    // Read file content into buffer
    fread(buffer, 1, length, file);
    buffer[length] = '\0';  // Null-terminate the string
    fclose(file);   // Close the file
    return buffer;  // Return the file content
}

// Main function to process the input file
int main(int argc, char *argv[]) {
    if(argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);  // Print usage message if incorrect number of arguments
        return -1;
    }

    char *source = read_file(argv[1]);
    const char *input = source;
    Token token;

    // Keep getting tokens until EOF is reached
    while((token = get_token(&input)).type != TOKEN_EOF) {
        printf("Token: Type=%d, Value='%s'\n", token.type, token.value);    // Print token type and value
    }

    free(source);   // Free the allocated memory
    return 0;
}
