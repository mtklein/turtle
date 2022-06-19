#include "buffer.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const size_t k = 10;
    struct buffer *b = NULL;

    for (size_t i = 0; i < k; i++) {
        b = buffer_grow(b, sizeof i);
        memcpy(b->bytes + i * sizeof i, &i, sizeof i);
    }
    assert(b->len == k * sizeof(size_t));

    for (size_t v,i = 0; i < k; i++) {
        memcpy(&v, b->bytes + i * sizeof i, sizeof v);
        assert(v == i);
    }

    free(b);
    return 0;
}
