#include "buffer.h"
#include "test.h"
#include <stdlib.h>

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
    double const goal_ns = argc > 1 ? atof(argv[1]) : 1e6;

    bench(goal_ns, growth);
    free(b);

    return 0;
}
