#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "lexer.h"
#include "parser.h"

typedef struct {
    Token* tokens;
    int tokenCount;
    int currentIndex;
} Parser;

void initParser(Parser* parser, Token* tokens, int tokenCount) {
    parser->tokens = tokens;
    parser->tokenCount = tokenCount;
    parser->currentIndex = 0;
}

Token getCurrentToken(Parser* parser) {
    if(parser->currentIndex < parser->tokenCount) {
        return parser->tokens[parser->currentIndex];
    } else {
        Token eofToken = {TOKEN_EOF, "EOF"};
        return eofToken;
    }
}

Token peekNextToken(Parser* parser) {
    if(parser->currentIndex + 1 < parser->tokenCount) {
        return parser->tokens[parser->currentIndex + 1];
    } else {
        Token eofToken = {TOKEN_EOF, "EOF"};
        return eofToken;
    }
}

void advance(Parser* parser) {
    parser->currentIndex++;
}

bool match(Parser* parser, TokenType type) {
    Token currentToken = getCurrentToken(parser);
    if (currentToken.type == type) {
        advance(parser);
        return true;
    }
    return false;
}

void backup(Parser* src, Parser* dest) {
    dest->tokens = src->tokens;
    dest->tokenCount = src->tokenCount;
    dest->currentIndex = src->currentIndex;
}

void restore(Parser* src, Parser* dest) {
    dest->currentIndex = src->currentIndex;
}

bool expect(Parser* parser, TokenType type) {
    if (match(parser, type)) {
        return true;
    }
    printf("Syntax error: Expected token type %d, got %d ('%s')\n", 
           type, getCurrentToken(parser).type, getCurrentToken(parser).value);
    return false;
}

// Forward declarations
bool parseExpr(Parser* parser);
bool parseExprAssign(Parser* parser);
bool parseExprOr(Parser* parser);
bool parseExprAnd(Parser* parser);
bool parseExprEq(Parser* parser);
bool parseExprRel(Parser* parser);
bool parseExprAdd(Parser* parser);
bool parseExprMul(Parser* parser);
bool parseExprCast(Parser* parser);
bool parseExprUnary(Parser* parser);
bool parseExprPostfix(Parser* parser);
bool parseExprPrimary(Parser* parser);
bool parseTypeName(Parser* parser);
bool parseStatement(Parser* parser);
bool parseBlock(Parser* parser);
bool parseDeclaration(Parser* parser);
bool parseVarDeclaration(Parser* parser);
bool parseFunctionDeclaration(Parser* parser);
bool parseForStatement(Parser* parser);
bool parseIfStatement(Parser* parser);
bool parseReturnStatement(Parser* parser);
bool parseExpressionStatement(Parser* parser);
bool parseProgram(Parser* parser);


bool isComment(Token token) {
    // Check if the token value starts with "//" or "/*"
    return (token.value[0] == '/' && 
           (token.value[1] == '/' || token.value[1] == '*'));
}

// Parse expression
bool parseExpr(Parser* parser) {
    return parseExprAssign(parser);
}

// Parse assignment expression
bool parseExprAssign(Parser* parser) {
    int startPos = parser->currentIndex;
    
    // Try to parse unary expression followed by assignment
    if (parseExprUnary(parser) && match(parser, TOKEN_ASSIGN)) {
        // Successfully parsed LHS and '=', now parse RHS
        if (parseExprAssign(parser)) {
            return true;
        } else {
            // If RHS parsing fails, backtrack
            parser->currentIndex = startPos;
            return false;
        }
    } else {
        // Backtrack and parse as logical OR expression
        parser->currentIndex = startPos;
        return parseExprOr(parser);
    }
}

// Parse OR expression
bool parseExprOr(Parser* parser) {
    if (!parseExprAnd(parser)) {
        return false;
    }
    
    while (getCurrentToken(parser).type == TOKEN_OR) {
        advance(parser);
        if (!parseExprAnd(parser)) {
            return false;
        }
    }
    
    return true;
}

// Parse AND expression
bool parseExprAnd(Parser* parser) {
    if (!parseExprEq(parser)) {
        return false;
    }
    
    while (getCurrentToken(parser).type == TOKEN_AND) {
        advance(parser);
        if (!parseExprEq(parser)) {
            return false;
        }
    }
    
    return true;
}

// Parse equality expression
bool parseExprEq(Parser* parser) {
    if (!parseExprRel(parser)) {
        return false;
    }
    
    while (getCurrentToken(parser).type == TOKEN_EQUAL || 
           getCurrentToken(parser).type == TOKEN_NOTEQUAL) {
        advance(parser);
        if (!parseExprRel(parser)) {
            return false;
        }
    }
    
    return true;
}

// Parse relational expression
bool parseExprRel(Parser* parser) {
    if (!parseExprAdd(parser)) {
        return false;
    }
    
    while (getCurrentToken(parser).type == TOKEN_LESS || 
           getCurrentToken(parser).type == TOKEN_LESSEQUAL ||
           getCurrentToken(parser).type == TOKEN_GREATER ||
           getCurrentToken(parser).type == TOKEN_GREATEREQUAL) {
        advance(parser);
        if (!parseExprAdd(parser)) {
            return false;
        }
    }
    
    return true;
}

// Parse addition/subtraction expression
bool parseExprAdd(Parser* parser) {
    if (!parseExprMul(parser)) {
        return false;
    }
    
    while (getCurrentToken(parser).type == TOKEN_PLUS || 
           getCurrentToken(parser).type == TOKEN_MINUS) {
        advance(parser);
        if (!parseExprMul(parser)) {
            return false;
        }
    }
    
    return true;
}

// Parse multiplication/division expression
bool parseExprMul(Parser* parser) {
    if (!parseExprCast(parser)) {
        return false;
    }
    
    while (getCurrentToken(parser).type == TOKEN_MULTIPLY || 
           getCurrentToken(parser).type == TOKEN_DIVIDE) {
        advance(parser);
        if (!parseExprCast(parser)) {
            return false;
        }
    }
    
    return true;
}

// Parse type casting expression
bool parseExprCast(Parser* parser) {
    if (getCurrentToken(parser).type == TOKEN_LPAREN) {
        int startPos = parser->currentIndex;
        
        // Try parsing '(' typeName ')' exprCast
        Parser backupParser;
        backup(parser, &backupParser);
        
        advance(&backupParser);  // Consume '('
        
        if (parseTypeName(&backupParser) && match(&backupParser, TOKEN_RPAREN)) {
            // If this path succeeds, continue with it
            restore(&backupParser, parser);
            advance(parser);  // Consume '('
            parseTypeName(parser);
            match(parser, TOKEN_RPAREN);
            return parseExprCast(parser);
        }
        
        // If the above fails, reset position
        parser->currentIndex = startPos;
    }
    
    return parseExprUnary(parser);
}

// Parse type name
bool parseTypeName(Parser* parser) {
    // Check for basic types: int, float, char, void
    if (getCurrentToken(parser).type == TOKEN_KEYWORD) {
        const char* keyword = getCurrentToken(parser).value;
        if (strcmp(keyword, "int") == 0 || 
            strcmp(keyword, "float") == 0 || 
            strcmp(keyword, "char") == 0 || 
            strcmp(keyword, "void") == 0 ||
            strcmp(keyword, "double") == 0) {
            advance(parser);
            return true;
        }
    } else if (getCurrentToken(parser).type == TOKEN_IDENTIFIER) {
        // Allow custom type names (structs, etc.)
        advance(parser);
        return true;
    }
    return false;
}

// Parse unary expression
bool parseExprUnary(Parser* parser) {
    if (getCurrentToken(parser).type == TOKEN_MINUS || 
        getCurrentToken(parser).type == TOKEN_NOT ||
        getCurrentToken(parser).type == TOKEN_PLUS_1 ||
        getCurrentToken(parser).type == TOKEN_MINUS_1) {
        advance(parser);
        return parseExprUnary(parser);
    }
    
    return parseExprPostfix(parser);
}

// Parse postfix expression (including array access and function calls)
bool parseExprPostfix(Parser* parser) {
    if (!parseExprPrimary(parser)) {
        return false;
    }
    
    while (true) {
        if (getCurrentToken(parser).type == TOKEN_LBRACKET) {
            // Handle array indexing
            advance(parser);
            if (!parseExpr(parser)) {
                return false;
            }
            if (!match(parser, TOKEN_RBRACKET)) {
                return false;
            }
        } else if (getCurrentToken(parser).type == TOKEN_DOT) {
            // Handle structure member access
            advance(parser);
            if (!match(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
        } else if (getCurrentToken(parser).type == TOKEN_PLUS_1 || 
                   getCurrentToken(parser).type == TOKEN_MINUS_1) {
            // Handle postfix increment/decrement
            advance(parser);
        } else {
            break;
        }
    }
    
    return true;
}

// Parse primary expression
bool parseExprPrimary(Parser* parser) {
    if (getCurrentToken(parser).type == TOKEN_IDENTIFIER) {
        printf("Found identifier: %s\n", getCurrentToken(parser).value);
        advance(parser);
        
        // Check for function call
        if (getCurrentToken(parser).type == TOKEN_LPAREN) {
            printf("Found function call\n");
            advance(parser);
            
            // Parse arguments if any
            if (getCurrentToken(parser).type != TOKEN_RPAREN) {
                if (!parseExpr(parser)) {
                    printf("Failed to parse function argument\n");
                    return false;
                }
                
                while (getCurrentToken(parser).type == TOKEN_COMMA) {
                    advance(parser);
                    if (!parseExpr(parser)) {
                        printf("Failed to parse function argument after comma\n");
                        return false;
                    }
                }
            }
            
            if (!match(parser, TOKEN_RPAREN)) {
                printf("Expected closing parenthesis in function call\n");
                return false;
            }
            printf("Successfully parsed function call\n");
        }
        
        return true;
    } else if (getCurrentToken(parser).type == TOKEN_NUMBER_ZEC || 
               getCurrentToken(parser).type == TOKEN_NUMBER_HEX ||
               getCurrentToken(parser).type == TOKEN_NUMBER_OCT ||
               getCurrentToken(parser).type == TOKEN_REAL ||
               getCurrentToken(parser).type == TOKEN_STRING ||
               getCurrentToken(parser).type == TOKEN_CHAR_LITERAL) {
        printf("Found string literal: %s\n", getCurrentToken(parser).value);
        advance(parser);
        return true;
    } else if (getCurrentToken(parser).type == TOKEN_LPAREN) {
        advance(parser);
        if (!parseExpr(parser)) {
            return false;
        }
        return match(parser, TOKEN_RPAREN);
    }
    
    return false;
}

// Parse statement
bool parseStatement(Parser* parser) {
    Token current = getCurrentToken(parser);
    printf("DEBUG: Parsing statement at token %d: %s (type %d)\n", 
           parser->currentIndex, current.value, current.type);
    
    // Block statement
    if (current.type == TOKEN_LBRACE) {
        return parseBlock(parser);
    }
    // For statement
    else if (current.type == TOKEN_KEYWORD && strcmp(current.value, "for") == 0) {
        return parseForStatement(parser);
    }
    // If statement
    else if (current.type == TOKEN_KEYWORD && strcmp(current.value, "if") == 0) {
        return parseIfStatement(parser);
    }
    // Return statement
    else if (current.type == TOKEN_KEYWORD && strcmp(current.value, "return") == 0) {
        return parseReturnStatement(parser);
    }
    // Declaration statement
    else if (current.type == TOKEN_KEYWORD && 
            (strcmp(current.value, "int") == 0 || 
             strcmp(current.value, "float") == 0 || 
             strcmp(current.value, "char") == 0 || 
             strcmp(current.value, "void") == 0 ||
             strcmp(current.value, "double") == 0)) {
        return parseDeclaration(parser);
    }
    // Skip comments
    else if (current.type == TOKEN_LINECOMMENT || current.type == TOKEN_MULTILINECOMMENT || 
             (current.value[0] == '/' && 
              (current.value[1] == '/' || current.value[1] == '*'))) {
        advance(parser);
        return true;
    }
    // Expression statement
    else {
        return parseExpressionStatement(parser);
    }
}


// Parse block of statements
bool parseBlock(Parser* parser) {
    if (!match(parser, TOKEN_LBRACE)) {
        printf("Expected opening brace for block, got token type %d: %s\n", 
               getCurrentToken(parser).type, getCurrentToken(parser).value);
        return false;
    }
    
    printf("Starting block at token %d\n", parser->currentIndex);
    
    // Parse statements until we hit the closing brace
    while (getCurrentToken(parser).type != TOKEN_RBRACE && 
           getCurrentToken(parser).type != TOKEN_EOF) {
        if (!parseStatement(parser)) {
            printf("Failed to parse statement in block at token %d: %s\n", 
                   parser->currentIndex, getCurrentToken(parser).value);
            return false;
        }
    }
    
    if (getCurrentToken(parser).type == TOKEN_EOF) {
        printf("Unexpected end of file in block\n");
        return false;
    }
    
    if (!match(parser, TOKEN_RBRACE)) {
        printf("Expected closing brace for block, got token type %d: %s\n", 
               getCurrentToken(parser).type, getCurrentToken(parser).value);
        return false;
    }
    
    printf("Completed block at token %d\n", parser->currentIndex);
    return true;
}

// Parse declaration
bool parseDeclaration(Parser* parser) {
    // Look ahead to see if this is a function declaration or a variable declaration
    int startPos = parser->currentIndex;
    printf("DEBUG: Trying to parse declaration at token %d: %s\n", 
        parser->currentIndex, getCurrentToken(parser).value);
    
    // Parse type name
    if (!parseTypeName(parser)) {
        printf("DEBUG: Failed to parse type name in declaration\n");
        return false;
    }
    
    // Check for special case of 'void main()'
    if (startPos < parser->tokenCount && 
        parser->tokens[startPos].type == TOKEN_KEYWORD && 
        strcmp(parser->tokens[startPos].value, "void") == 0) {
        
        if (parser->currentIndex < parser->tokenCount && 
            getCurrentToken(parser).type == TOKEN_IDENTIFIER && 
            strcmp(getCurrentToken(parser).value, "main") == 0) {
            
            // This is likely a main function declaration
            // Reset and parse as function
            parser->currentIndex = startPos;
            return parseFunctionDeclaration(parser);
        }
    }
    
    // Parse identifier
    if (!match(parser, TOKEN_IDENTIFIER)) {
        parser->currentIndex = startPos;
        return false;
    }
    
    // If next token is '(', this is a function declaration
    if (getCurrentToken(parser).type == TOKEN_LPAREN) {
        parser->currentIndex = startPos;
        return parseFunctionDeclaration(parser);
    }
    // Otherwise, it's a variable declaration
    else {
        parser->currentIndex = startPos;
        return parseVarDeclaration(parser);
    }
}

// Parse variable declaration
bool parseVarDeclaration(Parser* parser) {
    printf("DEBUG: Parsing variable declaration at token %d: %s\n", 
        parser->currentIndex, getCurrentToken(parser).value);
    // Parse type name
    if (!parseTypeName(parser)) {
        printf("DEBUG: Failed to parse type name\n");
        return false;
    }
    
    printf("DEBUG: After type name, at token %d: %s\n", 
        parser->currentIndex, getCurrentToken(parser).value);

    // Parse first variable
    if (!match(parser, TOKEN_IDENTIFIER)) {
        return false;
    }
    
    // Check for array declaration
    if (getCurrentToken(parser).type == TOKEN_LBRACKET) {
        advance(parser);
        // Optional array size
        if (getCurrentToken(parser).type == TOKEN_NUMBER_ZEC ||
            getCurrentToken(parser).type == TOKEN_NUMBER_HEX ||
            getCurrentToken(parser).type == TOKEN_NUMBER_OCT) {
            advance(parser);
        }
        if (!match(parser, TOKEN_RBRACKET)) {
            return false;
        }
    }
    
    // Parse additional variables
    while (getCurrentToken(parser).type == TOKEN_COMMA) {
        advance(parser);
        
        if (!match(parser, TOKEN_IDENTIFIER)) {
            return false;
        }
        
        // Check for array declaration
        if (getCurrentToken(parser).type == TOKEN_LBRACKET) {
            advance(parser);
            // Optional array size
            if (getCurrentToken(parser).type == TOKEN_NUMBER_ZEC ||
                getCurrentToken(parser).type == TOKEN_NUMBER_HEX ||
                getCurrentToken(parser).type == TOKEN_NUMBER_OCT) {
                advance(parser);
            }
            if (!match(parser, TOKEN_RBRACKET)) {
                return false;
            }
        }
    }
    
    // Expect semicolon
    return match(parser, TOKEN_SEMICOLON);
}

// Parse function declaration
bool parseFunctionDeclaration(Parser* parser) {
    // Parse return type
    if (!parseTypeName(parser)) {
        return false;
    }
    
    // Parse function name
    if (!match(parser, TOKEN_IDENTIFIER)) {
        return false;
    }
    
    // Parse parameter list
    if (!match(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    // Parse parameters if any
    if (getCurrentToken(parser).type != TOKEN_RPAREN) {
        // Parse first parameter
        if (!parseTypeName(parser)) {
            return false;
        }
        
        if (!match(parser, TOKEN_IDENTIFIER)) {
            return false;
        }
        
        // Parse additional parameters
        while (getCurrentToken(parser).type == TOKEN_COMMA) {
            advance(parser);
            
            if (!parseTypeName(parser)) {
                return false;
            }
            
            if (!match(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
        }
    }
    
    if (!match(parser, TOKEN_RPAREN)) {
        return false;
    }
    
    // Parse function body
    return parseBlock(parser);
}

// Parse for statement
bool parseForStatement(Parser* parser) {
    if (!match(parser, TOKEN_KEYWORD) || strcmp(parser->tokens[parser->currentIndex-1].value, "for") != 0) {
        return false;
    }
    
    if (!match(parser, TOKEN_LPAREN)) {
        return false;
    }
    
    // Parse initialization
    if (getCurrentToken(parser).type == TOKEN_KEYWORD) {
        // Declaration as initialization
        if (!parseVarDeclaration(parser)) {
            return false;
        }
    } else if (getCurrentToken(parser).type != TOKEN_SEMICOLON) {
        // Expression as initialization
        printf("Parsing initialization expression at token %d: %s\n", 
               parser->currentIndex, getCurrentToken(parser).value);
               
        // Try to parse an identifier first
        if (getCurrentToken(parser).type == TOKEN_IDENTIFIER) {
            advance(parser); // Consume identifier
            
            // Check for assignment operator
            if (getCurrentToken(parser).type == TOKEN_ASSIGN) {
                advance(parser); // Consume '='
                
                // Parse right-hand side of assignment
                if (!parseExpr(parser)) {
                    printf("Failed to parse RHS of assignment in for loop init\n");
                    return false;
                }
            } else {
                printf("Expected '=' after identifier in for loop init\n");
                return false;
            }
        } else {
            // Not an identifier, try regular expression parsing
            if (!parseExpr(parser)) {
                printf("Failed to parse expression in for loop init\n");
                return false;
            }
        }
        
        if (!match(parser, TOKEN_SEMICOLON)) {
            printf("Expected semicolon after initialization, got token %d: %s\n", 
                   parser->currentIndex, getCurrentToken(parser).value);
            return false;
        }
    } else {
        // Empty initialization
        advance(parser);
    }
    
    // Parse condition (can be empty)
    printf("Parsing for loop condition at token %d: %s\n", 
           parser->currentIndex, getCurrentToken(parser).value);
    if (getCurrentToken(parser).type != TOKEN_SEMICOLON) {
        if (!parseExpr(parser)) {
            printf("Failed to parse condition in for loop\n");
            return false;
        }
    }
    
    if (!match(parser, TOKEN_SEMICOLON)) {
        printf("Expected semicolon after condition in for loop\n");
        return false;
    }
    
    // Parse increment (can be empty)
    printf("Parsing for loop increment at token %d: %s\n", 
           parser->currentIndex, getCurrentToken(parser).value);
    if (getCurrentToken(parser).type != TOKEN_RPAREN) {
        if (!parseExpr(parser)) {
            printf("Failed to parse increment in for loop. Current token: %s\n", 
                   getCurrentToken(parser).value);
            return false;
        }
    }
    
    if (!match(parser, TOKEN_RPAREN)) {
        printf("Expected closing parenthesis after for loop components\n");
        return false;
    }
    
    // Parse body
    printf("Parsing for loop body at token %d: %s\n", 
           parser->currentIndex, getCurrentToken(parser).value);
    bool result = parseStatement(parser);
    if (!result) {
        printf("Failed to parse for loop body\n");
    } else {
        printf("Successfully parsed for loop body, now at token %d: %s\n", 
               parser->currentIndex, getCurrentToken(parser).value);
    }
    return result;
}

// Parse if statement
// Parse if statement
bool parseIfStatement(Parser* parser) {
    printf("Starting if statement parsing\n");
    if (getCurrentToken(parser).type != TOKEN_KEYWORD || 
        strcmp(getCurrentToken(parser).value, "if") != 0) {
        return false;
    }
    
    advance(parser); // Now advance past the 'if' token
    
    if (!match(parser, TOKEN_LPAREN)) {
        printf("Expected '(' after 'if'\n");
        return false;
    }
    
    printf("Parsing if condition\n");
    
    // Parse the condition expression with special handling for complex conditions
    //int conditionStart = parser->currentIndex;
    int parenCount = 1; // We already consumed one opening parenthesis
    
    // Find the matching closing parenthesis by counting parens
    while (parser->currentIndex < parser->tokenCount && parenCount > 0) {
        if (getCurrentToken(parser).type == TOKEN_LPAREN) {
            parenCount++;
        } else if (getCurrentToken(parser).type == TOKEN_RPAREN) {
            parenCount--;
        }
        
        if (parenCount > 0) {
            advance(parser);
        }
    }
    
    if (parenCount > 0) {
        printf("Unmatched parentheses in if condition\n");
        return false;
    }
    
    // Now consume the closing parenthesis
    advance(parser);
    
    printf("Parsing if body at token %d: %s (type %d)\n", 
           parser->currentIndex, getCurrentToken(parser).value, getCurrentToken(parser).type);
    
    // Parse if body
    if (!parseStatement(parser)) {
        printf("Failed to parse if body\n");
        // Try to recover - skip to "else" or next statement
        while (parser->currentIndex < parser->tokenCount && 
               (getCurrentToken(parser).type != TOKEN_KEYWORD || 
                strcmp(getCurrentToken(parser).value, "else") != 0) &&
               getCurrentToken(parser).type != TOKEN_SEMICOLON &&
               getCurrentToken(parser).type != TOKEN_RBRACE) {
            advance(parser);
        }
    }
    
    // Skip any comments that might appear between if body and else
    while (parser->currentIndex < parser->tokenCount &&
           (getCurrentToken(parser).type == TOKEN_LINECOMMENT || 
            getCurrentToken(parser).type == TOKEN_MULTILINECOMMENT ||
            isComment(getCurrentToken(parser)))) {
        advance(parser);
    }
    
    // Parse optional else
    if (parser->currentIndex < parser->tokenCount &&
        getCurrentToken(parser).type == TOKEN_KEYWORD && 
        strcmp(getCurrentToken(parser).value, "else") == 0) {
        printf("Found else clause\n");
        advance(parser);
        
        printf("Parsing else body at token %d: %s\n", 
               parser->currentIndex, getCurrentToken(parser).value);
        
        return parseStatement(parser);
    }
    
    return true;
}

// Parse return statement
bool parseReturnStatement(Parser* parser) {
    // First check if the current token is 'return' before advancing
    if (getCurrentToken(parser).type != TOKEN_KEYWORD || 
        strcmp(getCurrentToken(parser).value, "return") != 0) {
        return false;
    }
    
    // Now advance past the 'return' token
    advance(parser);
    
    // Parse optional return value
    if (getCurrentToken(parser).type != TOKEN_SEMICOLON) {
        if (!parseExpr(parser)) {
            printf("Failed to parse return value expression\n");
            return false;
        }
    }
    
    if (!match(parser, TOKEN_SEMICOLON)) {
        printf("Expected semicolon after return statement\n");
        return false;
    }
    
    return true;
}

// Parse expression statement
bool parseExpressionStatement(Parser* parser) {
    // Empty statement
    if (getCurrentToken(parser).type == TOKEN_SEMICOLON) {
        advance(parser);
        return true;
    }
    
    printf("Trying to parse expression statement at token %d: %s (type %d)\n", 
           parser->currentIndex, getCurrentToken(parser).value, getCurrentToken(parser).type);
    
    // Special case for function calls which are common in expression statements
    if (getCurrentToken(parser).type == TOKEN_IDENTIFIER) {
        int startPos = parser->currentIndex;
        
        // Save function name for debugging
        char functionName[256];
        strcpy(functionName, getCurrentToken(parser).value);
        advance(parser); // Consume function name
        
        if (getCurrentToken(parser).type == TOKEN_LPAREN) {
            printf("Parsing function call to %s\n", functionName);
            advance(parser); // Consume '('
            
            // Parse arguments if any
            if (getCurrentToken(parser).type != TOKEN_RPAREN) {
                // Handle special case of string literals that may be split across tokens
                if (getCurrentToken(parser).type == TOKEN_STRING || 
                    getCurrentToken(parser).type == TOKEN_CHAR_LITERAL) {
                    // Skip the string/char literal token
                    printf("Processing string/char literal argument\n");
                    advance(parser);
                    
                    // Skip any additional tokens that might be part of the string
                    // until we find the closing parenthesis
                    while (parser->currentIndex < parser->tokenCount && 
                           getCurrentToken(parser).type != TOKEN_RPAREN) {
                        printf("Skipping additional string token: %s\n", getCurrentToken(parser).value);
                        advance(parser);
                    }
                } else {
                    // Parse regular expression argument
                    if (!parseExpr(parser)) {
                        printf("Failed to parse function argument\n");
                        parser->currentIndex = startPos;
                        return false;
                    }
                }
            }
            
            if (!match(parser, TOKEN_RPAREN)) {
                printf("Expected closing parenthesis in function call\n");
                // Try to recover - find the next closing parenthesis
                while (parser->currentIndex < parser->tokenCount && 
                       getCurrentToken(parser).type != TOKEN_RPAREN) {
                    advance(parser);
                }
                if (getCurrentToken(parser).type == TOKEN_RPAREN) {
                    advance(parser); // Consume the closing parenthesis
                } else {
                    return false;
                }
            }
            
            // Expect semicolon after function call
            if (!match(parser, TOKEN_SEMICOLON)) {
                printf("Expected semicolon after function call\n");
                // Try to recover - find the next semicolon
                while (parser->currentIndex < parser->tokenCount && 
                       getCurrentToken(parser).type != TOKEN_SEMICOLON) {
                    advance(parser);
                }
                if (getCurrentToken(parser).type == TOKEN_SEMICOLON) {
                    advance(parser); // Consume the semicolon
                } else {
                    return false;
                }
            }
            
            printf("Successfully parsed function call to %s\n", functionName);
            return true;
        }
        
        // If not a function call, reset and try normal expression parsing
        parser->currentIndex = startPos;
    }
    
    // Try to parse a normal expression
    if (!parseExpr(parser)) {
        printf("Failed to parse expression in statement\n");
        return false;
    }
    
    printf("Expression parsed, expecting semicolon at token %d: %s\n", 
           parser->currentIndex, getCurrentToken(parser).value);
    
    // Expect semicolon at the end
    if (!match(parser, TOKEN_SEMICOLON)) {
        printf("Expected semicolon after expression statement\n");
        // Try to recover - skip to next semicolon
        while (parser->currentIndex < parser->tokenCount && 
               getCurrentToken(parser).type != TOKEN_SEMICOLON) {
            advance(parser);
        }
        if (getCurrentToken(parser).type == TOKEN_SEMICOLON) {
            advance(parser); // Consume the semicolon
            return true;     // Continue parsing
        }
        return false;
    }
    
    return true;
}



// Parse program (top-level constructs)
bool parseProgram(Parser* parser) {
    // Skip any comments at the beginning
    while (isComment(getCurrentToken(parser))) {
        advance(parser);
    }
    
    while (getCurrentToken(parser).type != TOKEN_EOF) {
        // Skip any comments in the middle of the code
        if (isComment(getCurrentToken(parser))) {
            advance(parser);
            continue;
        }
        
        if (!parseDeclaration(parser) && !parseStatement(parser)) {
            return false;
        }
    }
    return true;
}

// Main parse function that interfaces with the main.c file
int parse(Token* tokens, int token_count) {
    Parser parser;
    initParser(&parser, tokens, token_count);
    
    // Parse the entire program
    bool result = parseProgram(&parser);
    
    if (!result) {
        int errorPosition = parser.currentIndex;
        printf("Syntax error at token %d: %s\n", 
               errorPosition, 
               errorPosition < token_count ? tokens[errorPosition].value : "EOF");
    }
    
    return result ? 1 : 0;  // Return 1 for success, 0 for failure
}