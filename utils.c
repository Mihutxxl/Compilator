#include "utils.h"

void *safeAlloc(size_t nbytes) {
    void *p = malloc(nbytes);
    if (!p) {
        fprintf(stderr, "Error: out of memory\n");
        exit(EXIT_FAILURE);
    }
    return p;
}