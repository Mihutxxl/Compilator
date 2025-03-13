#ifndef SYNTACT_H
#define SYNTACT_H

#include "lexer.h"  // Include lexer.h to access TokenType and Token structure
#include <stdbool.h> // Include for bool type

// External variable declaration for tracking current token position
extern int currentIndex;

// Main parsing function
bool parse(Token* tokens, int tokenCount);

// Parser functions for specific grammar rules
bool parseDeclVar(Token* tokens, int tokenCount);
bool parseArrayDecl(Token* tokens, int tokenCount);

// Other function declarations if needed (like unit)
void unit(Token* tokens, int tokenCount);

#endif  // SYNTACT_H