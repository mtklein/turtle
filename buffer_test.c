#include "buffer.h"
#include "test.h"
#include <stdlib.h>

static void test(void) {
    const int k = 20000;

    int *b = NULL;
    for (int i = 0; i < k; i++) {
        b = buffer_push(b,i);
        b[i] = i;
    }
    for (int i = 0; i < k; i++) {
        expect(b[i] == i);
    }
    free(b);
}

static int    n;
static float *b;

static double growth(int loops) {
    while (loops --> 0) {
        b = buffer_push(b, n);
        b[n++] = 42;
    }
    return 1;
}

int main(int argc, char **argv) {
    bench_goal_ns = argc > 1 ? atof(argv[1]) : bench_goal_ns;

    test();
    bench(growth);
    free(b);

    return 0;
}
