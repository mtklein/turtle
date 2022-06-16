#include "array.h"
#include <limits.h>
#include <stdlib.h>

static size_t cap(size_t len) {
    return len <= 1 ? len
                    : 1 << (CHAR_BIT*sizeof(len) - (size_t)__builtin_clzll(len-1));
}

char* array_grow(array *a, size_t more) {
    if (a->len + more > cap(a->len)) {
        a->ptr = realloc(a->ptr, cap(a->len + more));
    }
    char *ptr = a->ptr + a->len;
    a->len += more;
    return ptr;
}
