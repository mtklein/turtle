#include "buffer.h"
#include <stdlib.h>

__attribute__((no_sanitize("unsigned-integer-overflow")))
static _Bool is_zero_or_pow2(size_t n) {
    return (n & (n-1)) == 0;
}

void* buffer_push_(void *ptr, size_t n, size_t elt) {
    return is_zero_or_pow2(n) ? realloc(ptr, (n ? 2*n : 1) * elt) : ptr;
}
