#include "array.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static void with_array(void (*fn)(array*)) {
    array a = {0};
    fn(&a);
    free(a.ptr);
}

static void test_growth(array *a) {
    const int k = 10;

    for (size_t i = 0; i < k; i++) {
        assert(a->len == i * sizeof i);
        memcpy(array_grow(a, sizeof i), &i, sizeof i);
    }

    for (size_t i = 0; i < k; i++) {
        size_t v;
        memcpy(&v, a->ptr + i * sizeof i, sizeof v);
        assert(v == i);
    }
}

int main(void) {
    with_array(test_growth);
    return 0;
}
