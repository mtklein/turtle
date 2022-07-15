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

static float *to_free;

static double growth(int loops) {
    float *buf = NULL;
    for (int i = 0; i < loops; i++) {
        buf = buffer_push(buf, i);
        buf[i] = 42;
    }
    to_free = buf;
    return 1;
}

int main(int argc, char **argv) {
    bench_goal_ns = argc > 1 ? atof(argv[1]) : bench_goal_ns;

    test();
    bench(growth);
    kill(to_free);

    return 0;
}
