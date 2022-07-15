#include "buffer.h"
#include <stdlib.h>

#if defined(__clang__)
__attribute__((no_sanitize("unsigned-integer-overflow")))
#endif
static _Bool not_pow2_or_zero(size_t x) {
    return x & (x-1);
}

void* buffer_push_(void *buf, size_t len, size_t sizeof_T) {
    return not_pow2_or_zero(len) ? buf
                                 : realloc(buf, (len ? 2*len : 1) * sizeof_T);
}
