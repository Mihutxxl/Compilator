#ifndef SYNTACT_H
#define SYNTACT_H

#include "lexer.h"  // Include lexer.h to access TokenType and Token structure
#include <stdbool.h> // Include for bool type

// Declare the function that parses the tokens
bool parse(Token* tokens, int tokenCount);

// Other function declarations if needed (like unit)
void unit(Token* tokens, int tokenCount);

#endif  // SYNTACT_H
