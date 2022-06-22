#include "expect.h"
#include <stdio.h>
#include <stdlib.h>

__attribute__((noreturn))
void expect_fail(const char* file, int line, const char* msg) {
    fprintf(stderr, "%s:%d expect(%s) failed\n", file, line, msg);
    abort();
}
