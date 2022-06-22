#include "buffer.h"
#include "expect.h"
#include <stdlib.h>
#include <string.h>

static void test_growth(void) {
    const size_t k = 10;
    struct buffer *b = NULL;

    for (size_t i = 0; i < k; i++) {
        b = buffer_grow(b, sizeof i);
        memcpy(b->bytes + i * sizeof i, &i, sizeof i);
    }
    expect(b->len == k * sizeof(size_t));

    for (size_t v,i = 0; i < k; i++) {
        memcpy(&v, b->bytes + i * sizeof i, sizeof v);
        expect(v == i);
    }

    free(b);
}

static void test_grow_by_zero(void) {
    struct buffer *b = NULL;
    for (int i = 0; i < 2; i++) {
        b = buffer_grow(b,0);
        expect(b == NULL);
    }
}

int main(void) {
    test_growth();
    test_grow_by_zero();
    return 0;
}
