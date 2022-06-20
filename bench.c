#include "bench.h"
#include "now.h"

double bench(double const goal_ns, double (*fn)(int loops)) {
    double elapsed_ns =  0,
           scale      =  1;
    int    lg_loops   = -1;

    while (elapsed_ns < goal_ns) {
        double const start = now_ns();
        scale = fn(1 << ++lg_loops);
        elapsed_ns = now_ns() - start;
    }

    return elapsed_ns / (1 << lg_loops) / scale;
}
