#include "bit_ceil.h"
#include "buffer.h"
#include <stdlib.h>

struct buffer* buffer_grow(struct buffer *b, size_t more) {
    size_t const len = b ? b->len : 0;
    if (bit_ceil(sizeof *b + len) < sizeof *b + len+more) {
        b = realloc(b, bit_ceil(sizeof *b + len+more));
    }
    if (b) {
        b->len = len+more;
    }
    return b;
}
