#include "now.h"
#include <sys/time.h>

double now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec*1e9 + ts.tv_nsec;
}
