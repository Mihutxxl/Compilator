#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "lexer.h"
#include "parcer.h"

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

bool parseExpr(Parser* parser) {
    return parseExprAssign(parser);
}

// Parse assignment expression
bool parseExprAssign(Parser* parser) {
    int startPos = parser->currentIndex;
    
    // Try to parse unary expression followed by assignment
    Parser backupParser;
    backup(parser, &backupParser);
    
    if (parseExprUnary(&backupParser) && match(&backupParser, TOKEN_ASSIGN)) {
        // If this path looks promising, continue with it
        restore(&backupParser, parser);
        if (parseExprUnary(parser) && match(parser, TOKEN_ASSIGN) && parseExprAssign(parser)) {
            return true;
        } else {
            return false;
        }
    }
    
    // Reset position and try exprOr
    parser->currentIndex = startPos;
    return parseExprOr(parser);
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

// Simple type name checking (simplified)
bool parseTypeName(Parser* parser) {
    if (getCurrentToken(parser).type == TOKEN_IDENTIFIER) {
        advance(parser);
        return true;
    }
    return false;
}

// Parse unary expression
bool parseExprUnary(Parser* parser) {
    if (getCurrentToken(parser).type == TOKEN_MINUS || 
        getCurrentToken(parser).type == TOKEN_NOT) {
        advance(parser);
        return parseExprUnary(parser);
    }
    
    return parseExprPostfix(parser);
}

// Parse postfix expression
bool parseExprPostfix(Parser* parser) {
    if (!parseExprPrimary(parser)) {
        return false;
    }
    
    while (true) {
        if (getCurrentToken(parser).type == TOKEN_LBRACKET) {
            advance(parser);
            if (!parseExpr(parser)) {
                return false;
            }
            if (!match(parser, TOKEN_RBRACKET)) {
                return false;
            }
        } else if (getCurrentToken(parser).type == TOKEN_DOT) {
            advance(parser);
            if (!match(parser, TOKEN_IDENTIFIER)) {
                return false;
            }
        } else {
            break;
        }
    }
    
    return true;
}

// Parse primary expression
bool parseExprPrimary(Parser* parser) {
    if (getCurrentToken(parser).type == TOKEN_IDENTIFIER) {
        advance(parser);
        
        // Check for function call
        if (getCurrentToken(parser).type == TOKEN_LPAREN) {
            advance(parser);
            
            // Parse arguments if any
            if (getCurrentToken(parser).type != TOKEN_RPAREN) {
                if (!parseExpr(parser)) {
                    return false;
                }
                
                while (getCurrentToken(parser).type == TOKEN_COMMA) {
                    advance(parser);
                    if (!parseExpr(parser)) {
                        return false;
                    }
                }
            }
            
            if (!match(parser, TOKEN_RPAREN)) {
                return false;
            }
        }
        
        return true;
    } else if (getCurrentToken(parser).type == TOKEN_NUMBER_ZEC || 
               getCurrentToken(parser).type == TOKEN_NUMBER_HEX ||
               getCurrentToken(parser).type == TOKEN_NUMBER_OCT ||
               getCurrentToken(parser).type == TOKEN_REAL ||
               getCurrentToken(parser).type == TOKEN_STRING ||
               getCurrentToken(parser).type == TOKEN_CHAR_LITERAL) {
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

// Main function to check if an expression follows assignment rule
bool isValidAssignmentExpression(Token* tokens, int tokenCount) {
    Parser parser;
    initParser(&parser, tokens, tokenCount);
    
    bool result = parseExprAssign(&parser);
    
    // Check if we've consumed all tokens
    if (result && (parser.currentIndex >= parser.tokenCount || 
                  getCurrentToken(&parser).type == TOKEN_EOF)) {
        return true;
    }
    
    return false;
}

// Main parse function that interfaces with the main.c file
int parse(Token* tokens, int token_count) {
    Parser parser;
    initParser(&parser, tokens, token_count);
    
    // Parse the entire expression
    bool result = parseExpr(&parser);
    
    // Check if we've consumed all tokens
    if (result && (parser.currentIndex >= parser.tokenCount || 
                  getCurrentToken(&parser).type == TOKEN_EOF)) {
        return 1;  // Success - return 1
    }
    
    return 0;  // Failure - return 0
}