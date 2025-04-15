#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Parse function that returns 1 if successful, 0 otherwise
int parse(Token* tokens, int token_count);

#endif // PARSER_H