#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parcer.h"

#define MAXVARS 100

int currentIndex = 0;
char* iden[MAXVARS];
int i = 0;

bool parseDeclVar(Token* tokens, int tokenCount);
bool parseDeclFunc(Token* tokens, int tokenCount);
bool parseArrayDecl(Token* tokens, int tokenCount);
bool parseExprAssign(Token* tokens, int tokenCount);

void consume(Token* tokens, int tokenCount) {
    if(currentIndex < tokenCount) {
        currentIndex++;
    }
}

Token getCurrentToken(Token* tokens) {
    return tokens[currentIndex];
}

bool parse(Token* tokens, int tokenCount) {
    currentIndex = 0;
    
    while(currentIndex < tokenCount) {
        Token current = tokens[currentIndex];

        if(current.type == TOKEN_KEYWORD) {
            int savedIndex = currentIndex;
            if(strcmp(current.value, "int") == 0) {
                currentIndex++;

                if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_IDENTIFIER) {
                    int idPos = currentIndex;
                    currentIndex++;

                    if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_LPAREN && strcmp(tokens[currentIndex].value, "(") == 0) {
                        currentIndex = savedIndex;
                        if(!parseDeclFunc(tokens, tokenCount)) {
                            printf("Error: Invalid function declaration\n");
                            return false;
                        }
                        continue;
                    }   else {
                        currentIndex = savedIndex;
                    }
                }
            }

            if(strcmp(current.value, "int") == 0) {
                currentIndex++;
                if(!parseDeclVar(tokens, tokenCount)) {
                    printf("Error: Invalid variable declaration\n");
                    return false;
                }
                continue;
            }
        } 
        if(tokens[currentIndex].type == TOKEN_IDENTIFIER) {
            int savedIndex = currentIndex;
            currentIndex++;

            if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_ASSIGN) {
                currentIndex++;
                if(currentIndex < tokenCount) {
                    currentIndex = savedIndex;
                    if(!parseExprAssign(tokens, tokenCount)) {
                        printf("Error: Invalid assignment\n");
                        return false;
                    }
                    continue;
                }
            }
        }
        currentIndex++;
    }

    for(int j = 0; j < i; j++) {
        printf("idens %s, ",iden[j]);
    }

    return true;
}

bool parseDeclVar(Token* tokens, int tokenCount) {
    // The current token should be an identifier
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
        return false;
    }
    iden[i++] = tokens[currentIndex].value;
    printf("Vaiable declaration: %s\n", tokens[currentIndex].value);
    currentIndex++; // Move past the identifier

    if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_LBRACKET && strcmp(tokens[currentIndex].value, "[") == 0) {
        if(!parseArrayDecl(tokens, tokenCount)) {
            return false;
        }
    }

    // Look for variables separated by comma
    while(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_COMMA && 
          strcmp(tokens[currentIndex].value, ",") == 0) {
        
        currentIndex++; // Skip the comma
        iden[i++] = tokens[currentIndex].value;
        // Next token must be an identifier
        if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
            return false;
        }
        
        printf("Variable declaration: %s\n", tokens[currentIndex].value);
        currentIndex++; // Move past the identifier

        if(currentIndex < tokenCount && tokens[currentIndex].type == TOKEN_LBRACKET && strcmp(tokens[currentIndex].value, "[") == 0) {
            if(!parseArrayDecl(tokens, tokenCount)) {
                return false;
            }
        }
    }
    // Check for the semicolon
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_SEMICOLON || strcmp(tokens[currentIndex].value, ";") != 0) {
        return false;
    }

    currentIndex++;
    return true;
}

void unit(Token* tokens, int tokenCount) {
    while(currentIndex < tokenCount) {
        Token current = getCurrentToken(tokens);

        if(current.type == TOKEN_KEYWORD) {
            if(strcmp(current.value, "int") == 0) {
                printf("Integer declaration\n");
            }
        }
        consume(tokens, tokenCount);
    }
}

bool parseArrayDecl(Token* tokens, int tokenCount) {
    currentIndex++;

    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_NUMBER_ZEC) {
        return false;
    }

    printf("Array size: %s\n", tokens[currentIndex].value);
    currentIndex++;

    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_RBRACKET || strcmp(tokens[currentIndex].value, "]") != 0) {
        return false;
    }

    currentIndex++;
    return true;
}

bool parseDeclFunc(Token* tokens, int tokenCount) {
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_KEYWORD) {
        return false;
    }

    const char* returnType = tokens[currentIndex].value;
    printf("Function return type: %s\n", returnType);
    currentIndex++;

    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
        return false;
    }

    const char* funcName = tokens[currentIndex].value;
    printf("Function name: %s\n", funcName);
    currentIndex++;

    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_LPAREN || strcmp(tokens[currentIndex].value, "(") != 0) {
        return false;
    }
    currentIndex++;

    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_RPAREN || strcmp(tokens[currentIndex].value, ")") != 0) {
        return false;
    }
    currentIndex++;
    return true;
}

bool parseExprAssign(Token* tokens, int tokenCount) {
    char* val;
    int ok = 0;
    strcpy(val, "");
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
        return false;
    }
    printf("Variable assign: %s\n", tokens[currentIndex].value);
    currentIndex++;
    if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_ASSIGN) {
        return false;
    }
    currentIndex++;
    //printf("%s\n", tokens[currentIndex].value);
    while((tokens[currentIndex].type == TOKEN_ASSIGN || tokens[currentIndex].type == TOKEN_IDENTIFIER || tokens[currentIndex].type == TOKEN_NUMBER_OCT || tokens[currentIndex].type == TOKEN_NUMBER_ZEC || tokens[currentIndex].type == TOKEN_PLUS || tokens[currentIndex].type == TOKEN_PLUS_1 || tokens[currentIndex].type == TOKEN_MINUS || tokens[currentIndex].type == TOKEN_MINUS_1 || tokens[currentIndex].type == TOKEN_LBRACKET)) {
        //printf("%s\n", tokens[currentIndex].value);
        //val = tokens[currentIndex].value;
        if(tokens[currentIndex].type == TOKEN_LBRACKET) {
            ok = 1;
            strcat(val, tokens[currentIndex].value);
            currentIndex++;
            while(tokens[currentIndex].type == TOKEN_IDENTIFIER || tokens[currentIndex].type == TOKEN_NUMBER_ZEC || tokens[currentIndex].type == TOKEN_NUMBER_OCT || tokens[currentIndex].type == TOKEN_PLUS || tokens[currentIndex].type == TOKEN_PLUS_1 || tokens[currentIndex].type == TOKEN_MINUS || tokens[currentIndex].type == TOKEN_MINUS_1) {
                ok = 2;
                strcat(val, tokens[currentIndex].value);
                currentIndex++;
            }
            if(tokens[currentIndex].type == TOKEN_RBRACKET) {
                strcat(val, tokens[currentIndex].value);
                currentIndex++;
            }

        }
        else {
            strcat(val, tokens[currentIndex].value);
            currentIndex++;
        }
    }
    //currentIndex++;
    if(currentIndex >= tokenCount || (tokens[currentIndex].type != TOKEN_SEMICOLON && tokens[currentIndex].type != TOKEN_RPAREN)) {
        return false;
    }
    printf("Value: %s\n", val);
    return true;
}

// bool parseAssignVal(Token* tokens, int tokenCount) {
//     int k = 0;
//     char* val;
//     //printf("1 %d\n",k);
//     if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_IDENTIFIER) {
//         return false;
//     }
//     // printf("TokenVal: %d\n", currentIndex);
//     printf("Variable assign: %s\n", tokens[currentIndex].value);
//     currentIndex++;
//     // printf("TokenVal: %d\n", currentIndex);
//     if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_ASSIGN) {
//         return false;
//     }
//     currentIndex++;
//     // printf("TokenVal: %d\n", currentIndex);
//     if(currentIndex >= tokenCount || (tokens[currentIndex].type != TOKEN_NUMBER_OCT && tokens[currentIndex].type != TOKEN_IDENTIFIER)) {
//         return false;
//         //k = 1;
//     }
//     if(tokens[currentIndex].type == TOKEN_IDENTIFIER){
//         k = 1;
//     }
//     val = tokens[currentIndex].value;
//     printf("2 %d\n",k);
//     if(k == 1) {
//         printf("TokenVal: %d\n", currentIndex);
//         currentIndex++;
//         printf("TokenVal: %d\n", currentIndex);
//         if(currentIndex >= tokenCount || (tokens[currentIndex].type != TOKEN_MINUS && tokens[currentIndex].type != TOKEN_PLUS && tokens[currentIndex].type != TOKEN_IDENTIFIER)) {
//             return false;
//         }
//         //printf("TokenVal: %d\n", currentIndex);
//         strcat(val, tokens[currentIndex].value);
//         currentIndex++;
//         if(currentIndex >= tokenCount || (tokens[currentIndex].type != TOKEN_NUMBER_ZEC && tokens[currentIndex].type != TOKEN_IDENTIFIER)) {
//             return false;
//         }
//         strcat(val, tokens[currentIndex].value);
//         currentIndex++;
//         if(tokens[currentIndex].type == TOKEN_LBRACKET) {
//             k = 2;
//         }
//         //printf("3 %d", k);
//         if(k == 2) {
//             if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_LBRACKET) {
//                 return false;
//             }
//             strcat(val, tokens[currentIndex].value);
//             currentIndex++;
//             if(currentIndex >= tokenCount || (tokens[currentIndex].type != TOKEN_IDENTIFIER && tokens[currentIndex].type != TOKEN_NUMBER_ZEC && strcmp(tokens[currentIndex].value, "0") != 0)) {
//                 return false;
//             }
//             strcat(val, tokens[currentIndex].value);
//             currentIndex++;
//             if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_RBRACKET) {
//                 return false;
//             }
//             strcat(val, tokens[currentIndex].value);
//             currentIndex++;
//             if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_SEMICOLON) {
//                 return false;
//             }
//         }
//         currentIndex++;
        
//         //if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_LBRACKET) {
//         //    return false;
//         //}
//         //strcat(val, tokens[currentIndex].value);
//         //currentIndex++;
//         printf("Value %s\n", val);
//     }
//     //printf("3 %d\n",k);
//     currentIndex++;
//     //printf("TokenVal: %d\n", currentIndex);
//     if(currentIndex >= tokenCount || tokens[currentIndex].type != TOKEN_SEMICOLON) {
//         return false;
//     }
//     currentIndex--;
//     if(k == 0) {
//         printf("Value: %s\n", tokens[currentIndex].value);
//     }
//     currentIndex++;
//     return true;
// }