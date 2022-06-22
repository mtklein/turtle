#if defined(UNITY_BUILD)
    #include "buffer.c"
    #include "expect.c"
#else
    #include "buffer.h"
    #include "expect.h"
#endif

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

#if defined(UNITY_BUILD)
static void test_cap(void) {
    expect(cap(0) == 0);
    expect(cap(1) == 1);
    expect(cap(2) == 2);
    expect(cap(3) == 4);
    for (size_t lg = 2; lg < CHAR_BIT * sizeof lg; lg++) {
        size_t pow2 = (size_t)1 << lg;
        expect(cap(pow2-1) == pow2);
    }
}
#endif

int main(void) {
    test_growth();
    test_grow_by_zero();
#if defined(UNITY_BUILD)
    test_cap();
#endif
    return 0;
}
