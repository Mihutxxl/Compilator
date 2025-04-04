#ifndef PARCER_H
#define PARCER_H

#include "lexer.h"

// Parse function that returns 1 if successful, 0 otherwise
int parse(Token* tokens, int token_count);

#endif // PARCER_H