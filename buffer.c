#include "buffer.h"
#include <stdlib.h>

__attribute__((no_sanitize("unsigned-integer-overflow")))
static _Bool is_zero_or_pow2(size_t x) {
    return (x & (x-1)) == 0;
}

void* buffer_push_(void *buf, size_t len, size_t sizeof_T) {
    return is_zero_or_pow2(len) ? realloc(buf, (len ? 2*len : 1) * sizeof_T) : buf;
}
