#include "bench.h"
#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>

#define kill(ptr) free(ptr), ptr=NULL

static struct buffer *b;

static double growth(int loops) {
    while (loops --> 0) {
        b = buffer_grow(b, sizeof(void*));
    }
    return 1;
}

int main(int argc, char **argv) {
    double const goal_ns = argc > 1 ? atof(argv[1]) : 1e6;

    {
        printf("growth\t%.2gns\n", bench(goal_ns, growth));
        kill(b);
    }

    return 0;
}
