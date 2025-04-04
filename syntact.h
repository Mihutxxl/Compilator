#ifndef PARCER_H
#define PARCER_H

#include "lexer.h"
#include <stdbool.h>

extern int currentIndex;

bool parse(Token* tokens, int tokenCount);

bool parseDeclvar(Token* tokens, int tokenCount);
bool parseDeclFunc(Token* tokens, int tokenCount);
bool parseArrayDecl(Token* tokens, int tokenCount);
bool parseExprAssign(Token* tokens, int tokenCount);

void unit(Token* tokens, int tokenCount);

#endif 