#include "test.h"
#include <stdio.h>
#include <time.h>

void expect_fail(char const *file, int line, char const *msg) {
    fprintf(stderr, "%s:%d expect(%s) failed\n", file, line, msg);
}

double bench_goal_ns = 1e6;

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9
         + (double)ts.tv_nsec;
}

void bench(char const *label, double (*fn)(int loops)) {
    double elapsed_ns =  0,
           scale      =  1;
    int    lg_loops   = -1;

    while (elapsed_ns < bench_goal_ns) {
        double const start = now_ns();
        scale = fn(1 << ++lg_loops);
        elapsed_ns = now_ns() - start;
    }

    fprintf(stdout, "%s\t%.2g\n", label, elapsed_ns / (1 << lg_loops) / scale);
}
