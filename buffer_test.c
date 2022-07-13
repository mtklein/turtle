#include "buffer.h"
#include "test.h"

static void test(void) {
    const int k = 20000;

    int *buf = NULL;
    for (int i = 0; i < k; i++) {
        buf = buffer_push(buf, i);
        buf[i] = i;
    }
    for (int i = 0; i < k; i++) {
        expect(buf[i] == i);
    }
    kill(buf);
}

static float *buf;
static int    len;

static double growth(int loops) {
    while (loops --> 0) {
        buf = buffer_push(buf, len);
        buf[len++] = 42;
    }
    return 1;
}

int main(int argc, char **argv) {
    bench_goal_ns = argc > 1 ? atof(argv[1]) : bench_goal_ns;

    test();
    bench(growth);
    kill(buf);

    return 0;
}
