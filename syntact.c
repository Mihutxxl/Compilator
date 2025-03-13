#include "lexer.h"  // Include lexer.h to access TokenType and Token structure
#include "syntact.h"
#include <stdio.h>
#include <stdbool.h>  // Include for bool type
#include <string.h>

// Global variable to store the index of the current token (for parsing)
int currentIndex = 0;

bool parseDeclVar(Token* tokens, int tokenCount);
bool parseArrayDecl(Token* tokens, int tokenCount);
bool parseDeclFunc(Token* tokens, int tokenCount);
bool parseFuncArg(Token* tokens, int tokenCount);
bool parseStmCompound(Token* tokens, int tokenCount);

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

// declVar: typeBase ID arrayDecl? (COMMA ID arrayDecl?)* SEMICOLON;
bool parse(Token* tokens, int tokenCount) {
    currentIndex = 0; // Make sure we start from the beginning
    
    while(currentIndex < tokenCount) {
        Token current = tokens[currentIndex];
        
        if(current.type == TOKEN_KEYWORD) {
            // Store the current position in case we need to backtrack
            int savedIndex = currentIndex;
            
            // Check for function declarations: typeBase MUL? | VOID
            if(strcmp(current.value, "int") == 0 || 
               strcmp(current.value, "float") == 0 || 
               strcmp(current.value, "char") == 0 || 
               strcmp(current.value, "bool") == 0 ||
               strcmp(current.value, "void") == 0) {
                
                currentIndex++;  // Move past the type
                
                // Check for optional MUL (pointer)
                if(currentIndex < tokenCount && 
                   tokens[currentIndex].type == TOKEN_MULTIPLY && 
                   strcmp(tokens[currentIndex].value, "*") == 0) {
                    currentIndex++;  // Move past the asterisk
                }
                
                // Look ahead to see if this is a function declaration
                if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_IDENTIFIER) {
                    // Save the identifier position
                    int idPos = currentIndex;
                    currentIndex++;
                    
                    // If we see a left parenthesis, it's a function declaration
                    if(currentIndex < tokenCount && 
                       tokens[currentIndex].type == TOKEN_LPAREN && 
                       strcmp(tokens[currentIndex].value, "(") == 0) {
                        
                        // Reset to the type position and parse as a function
                        currentIndex = savedIndex;
                        if(!parseDeclFunc(tokens, tokenCount)) {
                            printf("Error: Invalid function declaration\n");
                            return false;
                        }
                        continue;
                    } else {
                        // Not a function, reset to the type position and parse as a variable
                        currentIndex = savedIndex;
                    }
                }
            }
            
            // Check if it's a type base (for variable declarations)
            if(strcmp(current.value, "int") == 0 || strcmp(current.value, "float") == 0 || 
               strcmp(current.value, "char") == 0 || strcmp(current.value, "bool") == 0) {
                
                // We found a type base, now advance to the next token
                currentIndex++;
                
                // Parse declVar
                if(!parseDeclVar(tokens, tokenCount)) {
                    printf("Error: Invalid variable declaration\n");
                    return false;
                }
                
                continue; // Skip the advance at the end of the loop
            }
        }
        
        currentIndex++;
    }
    
    return true;
}

bool parseDeclFunc(Token* tokens, int tokenCount) {
    // Parse the return type (typeBase MUL? | VOID)
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_KEYWORD) {
        return false;
    }
    
    const char* returnType = tokens[currentIndex].value;
    printf("Function return type: %s\n", returnType);
    currentIndex++; // Move past the type
    
    // Check for optional MUL (pointer return type)
    bool isPointer = false;
    if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_MULTIPLY && 
       strcmp(tokens[currentIndex].value, "*") == 0) {
        isPointer = true;
        printf("Function returns pointer\n");
        currentIndex++; // Move past the asterisk
    }
    
    // Parse function name (ID)
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
        return false;
    }
    
    const char* functionName = tokens[currentIndex].value;
    printf("Function name: %s\n", functionName);
    currentIndex++; // Move past the function name
    
    // Parse LPAR - left parenthesis
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_LPAREN || 
       strcmp(tokens[currentIndex].value, "(") != 0) {
        return false;
    }
    currentIndex++; // Move past the left parenthesis
    
    // Parse optional function arguments
    if(currentIndex < tokenCount && !(tokens[currentIndex].type == TOKEN_RPAREN && 
       strcmp(tokens[currentIndex].value, ")") == 0)) {
        
        // Parse first argument
        if(!parseFuncArg(tokens, tokenCount)) {
            return false;
        }
        
        // Parse additional arguments separated by commas
        while(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_COMMA && 
              strcmp(tokens[currentIndex].value, ",") == 0) {
            
            currentIndex++; // Skip the comma
            
            // Parse next argument
            if(!parseFuncArg(tokens, tokenCount)) {
                return false;
            }
        }
    }
    
    // Parse RPAR - right parenthesis
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_RPAREN || 
       strcmp(tokens[currentIndex].value, ")") != 0) {
        return false;
    }
    currentIndex++; // Move past the right parenthesis
    
    // Parse compound statement (function body)
    if(!parseStmCompound(tokens, tokenCount)) {
        return false;
    }
    
    return true;
}

bool parseFuncArg(Token* tokens, int tokenCount) {
    // Parse argument type (typeBase)
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_KEYWORD) {
        return false;
    }
    
    const char* argType = tokens[currentIndex].value;
    currentIndex++; // Move past the type
    
    // Check for optional MUL (pointer parameter)
    bool isPointer = false;
    if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_MULTIPLY && 
       strcmp(tokens[currentIndex].value, "*") == 0) {
        isPointer = true;
        currentIndex++; // Move past the asterisk
    }
    
    // Parse parameter name (ID)
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
        return false;
    }
    
    const char* paramName = tokens[currentIndex].value;
    printf("Parameter: %s %s%s\n", argType, isPointer ? "*" : "", paramName);
    currentIndex++; // Move past the parameter name
    
    // Check for optional array declaration
    if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_LBRACKET && 
       strcmp(tokens[currentIndex].value, "[") == 0) {
        
        // Parse array declaration
        if(!parseArrayDecl(tokens, tokenCount)) {
            return false;
        }
    }
    
    return true;
}

bool parseStmCompound(Token* tokens, int tokenCount) {
    // Parse left brace '{'
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_LBRACE || 
       strcmp(tokens[currentIndex].value, "{") != 0) {
        return false;
    }
    
    printf("Function body start\n");
    currentIndex++; // Move past the left brace
    
    // Parse statements and declarations inside the function body
    int braceCount = 1;
    
    while(currentIndex < tokenCount && braceCount > 0) {
        // Try to parse variable declarations inside the function body
        if(tokens[currentIndex].type == TOKEN_KEYWORD) {
            if(strcmp(tokens[currentIndex].value, "int") == 0 || 
               strcmp(tokens[currentIndex].value, "float") == 0 || 
               strcmp(tokens[currentIndex].value, "char") == 0 || 
               strcmp(tokens[currentIndex].value, "bool") == 0) {
                
                // Found a type identifier, try to parse as a variable declaration
                int savedPos = currentIndex;
                currentIndex++; // Move past the type
                
                if(parseDeclVar(tokens, tokenCount)) {
                    // Successfully parsed a variable declaration, continue
                    continue;
                } else {
                    // Failed to parse as variable declaration, restore position
                    currentIndex = savedPos;
                }
            }
        }
        
        // Handle brace counting
        if(tokens[currentIndex].type == TOKEN_LBRACE) {
            braceCount++;
        } else if(tokens[currentIndex].type == TOKEN_RBRACE) {
            braceCount--;
        }
        
        // For now, just log what we're skipping
        //if(tokens[currentIndex].type == TOKEN_IDENTIFIER) {
        //    printf("Skipping identifier: %s\n", tokens[currentIndex].value);
        //} else if(tokens[currentIndex].type == TOKEN_KEYWORD) {
        //    printf("Skipping keyword: %s\n", tokens[currentIndex].value);
        //} else {
        //    printf("Skipping token: %s\n", tokens[currentIndex].value);
        //}

        if(!parseDeclVar(tokens, tokenCount)) {
            printf("Error: Invalid variable declaration\n");
            return false;
        }
        
        // Move to next token
        currentIndex++;
        
        // If we've reached the matching closing brace
        if(braceCount == 0) {
            printf("Function body end\n");
            return true;
        }
    }
    
    // If we get here, we didn't find the matching closing brace
    return false;
}

bool parseDeclVar(Token* tokens, int tokenCount) {
    // We expect the current token to be an identifier
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
        return false;
    }
    
    printf("Variable declaration: %s\n", tokens[currentIndex].value);
    currentIndex++; // Move past the identifier
    
    // Check for optional array declaration
    if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_LBRACKET && 
       strcmp(tokens[currentIndex].value, "[") == 0) {
        
        // Parse array declaration
        if(!parseArrayDecl(tokens, tokenCount)) {
            return false;
        }
    }
    
    // Look for more variables separated by commas
    while(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_COMMA && 
          strcmp(tokens[currentIndex].value, ",") == 0) {
        
        currentIndex++; // Skip the comma
        
        // Next token must be an identifier
        if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
            return false;
        }
        
        printf("Variable declaration: %s\n", tokens[currentIndex].value);
        currentIndex++; // Move past the identifier
        
        // Check for optional array declaration
        if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_LBRACKET && 
           strcmp(tokens[currentIndex].value, "[") == 0) {
            
            // Parse array declaration
            if(!parseArrayDecl(tokens, tokenCount)) {
                return false;
            }
        }
    }
    
    // Finally, we should see a semicolon
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_SEMICOLON || 
       strcmp(tokens[currentIndex].value, ";") != 0) {
        return false;
    }
    
    currentIndex++; // Skip the semicolon
    return true;
}

bool parseArrayDecl(Token* tokens, int tokenCount) {
    // We're assuming the current token is '['
    currentIndex++; // Skip '['
    
    // Check for either a numeric constant or an identifier
    if(currentIndex >= tokenCount || 
       (tokens[currentIndex].type != TOKEN_NUMBER_ZEC || 
        tokens[currentIndex].type == TOKEN_IDENTIFIER)) {
        return false;
    }
    
    printf("Array size: %s\n", tokens[currentIndex].value);
    currentIndex++; // Skip the constant or identifier
    
    // Next token should be ']'
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_RBRACKET || 
       strcmp(tokens[currentIndex].value, "]") != 0) {
        return false;
    }
    
    currentIndex++; // Skip ']'
    return true;
}

// bool parse(Token* tokens, int tokenCount) {
//     // Example: Ensure the index is valid and check the current token type
//     while (currentIndex < tokenCount) {
//         Token current = getCurrentToken(tokens);  // Fetch the current token

//         // Example logic for parsing
//         if (current.type == TOKEN_IDENTIFIER) {
//             // Handle IDENTIFIER token
//             printf("Identifier: %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_NUMBER_ZEC) {
//             // Handle NUMBER_ZEC token
//             printf("Number (decimal): %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_NUMBER_HEX) {
//             // Handle HEX number token
//             printf("Number (hex): %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_KEYWORD) {
//             // Handle KEYWORD token
//             //unit(tokens, tokenCount);
//             printf("Keyword: %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_PLUS) {
//             // Handle PLUS token
//             printf("Plus operator: %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_MINUS) {
//             // Handle MINUS token
//             printf("Minus operator: %s\n", current.value);
//         }
//         else if (current.type == TOKEN_SEMICOLON) {
//             // Handle SEMICOLON token
//             printf("Semicolon: %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_LPAREN) {
//             // Handle LPAREN token
//             printf("Left Parenthesis: %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_RPAREN) {
//             // Handle RPAREN token
//             printf("Right Parenthesis: %s\n", current.value);
//         } 
//         else if (current.type == TOKEN_EOF) {
//             // End of file reached
//             break;
//         }

//         // Example: handle other token types accordingly, and advance to the next token
//         advance(tokens, tokenCount);
//     }

//     return true; // Indicate that parsing was successful
// }

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
