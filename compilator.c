#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parcer.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return -1;
    }

    int token_count = 0;
    Token *tokens = tokenize_file(argv[1], &token_count);

    if (!tokens) {
        printf("Tokenization failed!\n");
        return -1;
    }

    printf("Tokens received from lexer:\n");
    for (int i = 0; i < token_count; i++) {
        printf("Token %d: Type=%d, Value='%s'\n", i + 1, tokens[i].type, tokens[i].value);
    }

    // Call the syntactic analyzer
    if (!parse(tokens, token_count)) {
        printf("Syntax analysis failed!\n");
        free(tokens);
        return -1;
    }

    printf("Syntax analysis successful!\n");
    free(tokens);  // Free allocated memory
    return 0;
}
