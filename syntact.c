#include "lexer.h"  // Include lexer.h to access TokenType and Token structure
#include "syntact.h"
#include <stdio.h>
#include <stdbool.h>  // Include for bool type
#include <string.h>

// Global variable to store the index of the current token (for parsing)
static int currentIndex = 0;

// Function to get the current token from the token array
Token getCurrentToken(Token* tokens) {
    return tokens[currentIndex];
}

// Function to advance to the next token
void advance(Token* tokens, int tokenCount) {
    if (currentIndex < tokenCount) {
        currentIndex++;  // Increment the index to point to the next token in the array
    }
}

// Function to parse the tokens
bool parse(Token* tokens, int tokenCount) {
    // Example: Ensure the index is valid and check the current token type
    while (currentIndex < tokenCount) {
        Token current = getCurrentToken(tokens);  // Fetch the current token

        // Example logic for parsing
        if (current.type == TOKEN_IDENTIFIER) {
            // Handle IDENTIFIER token
            printf("Identifier: %s\n", current.value);
        } 
        else if (current.type == TOKEN_NUMBER_ZEC) {
            // Handle NUMBER_ZEC token
            printf("Number (decimal): %s\n", current.value);
        } 
        else if (current.type == TOKEN_NUMBER_HEX) {
            // Handle HEX number token
            printf("Number (hex): %s\n", current.value);
        } 
        else if (current.type == TOKEN_KEYWORD) {
            // Handle KEYWORD token
            printf("Keyword: %s\n", current.value);
        } 
        else if (current.type == TOKEN_PLUS) {
            // Handle PLUS token
            printf("Plus operator: %s\n", current.value);
        } 
        else if (current.type == TOKEN_MINUS) {
            // Handle MINUS token
            printf("Minus operator: %s\n", current.value);
        }
        else if (current.type == TOKEN_SEMICOLON) {
            // Handle SEMICOLON token
            printf("Semicolon: %s\n", current.value);
        } 
        else if (current.type == TOKEN_LPAREN) {
            // Handle LPAREN token
            printf("Left Parenthesis: %s\n", current.value);
        } 
        else if (current.type == TOKEN_RPAREN) {
            // Handle RPAREN token
            printf("Right Parenthesis: %s\n", current.value);
        } 
        else if (current.type == TOKEN_EOF) {
            // End of file reached
            break;
        }

        // Example: handle other token types accordingly, and advance to the next token
        advance(tokens, tokenCount);
    }

    return true; // Indicate that parsing was successful
}

// Unit function to analyze the syntax for a basic statement or declaration
void unit(Token* tokens, int tokenCount) {
    while (currentIndex < tokenCount) {
        Token current = getCurrentToken(tokens);

        if (current.type == TOKEN_KEYWORD) {
            if (strcmp(current.value, "int") == 0) {
                // Process integer declaration or assignment
                printf("Integer declaration or assignment\n");
            } 
            else if (strcmp(current.value, "char") == 0) {
                // Process char declaration or assignment
                printf("Character declaration or assignment\n");
            } 
            else if (strcmp(current.value, "void") == 0) {
                // Process void function declaration
                printf("Void function declaration\n");
            }
        }

        // Handle other cases in the parsing process here
        advance(tokens, tokenCount);  // Proceed to the next token
    }
}
