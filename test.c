#include "test.h"
#include <time.h>

double bench_goal_ns = 1e6;

static double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1e9
         + (double)ts.tv_nsec;
}

double bench_(double (*fn)(int loops)) {
    double elapsed_ns =  0,
           scale      =  1;
    int    lg_loops   = -1;

    while (elapsed_ns < bench_goal_ns) {
        double const start = now_ns();
        scale = fn(1 << ++lg_loops);
        elapsed_ns = now_ns() - start;
    }

    return elapsed_ns / (1 << lg_loops) / scale;
}