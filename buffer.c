#undef UNITY_BUILD
#include "buffer.h"
#include <limits.h>
#include <stdlib.h>

static size_t cap(size_t len) {
    return len <= 1 ? len
                    : (size_t)1 << (CHAR_BIT*sizeof(len) - (size_t)__builtin_clzll(len-1));
}

struct buffer* buffer_grow(struct buffer *b, size_t more) {
    size_t const len = b ? b->len : 0;
    if (cap(len) < len+more) {
        b = realloc(b, sizeof *b + cap(len+more));
    }
    if (b) {
        b->len = len+more;
    }
    return b;
}
