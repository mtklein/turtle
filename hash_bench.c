#include "bench.h"
#include "expect.h"
#include "hash.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define kill(ptr) free(ptr), ptr=NULL

static struct hash *h;
static int         scale;

static double growth(int loops) {
    while (loops --> 0) {
        for (int i = 0; i < scale; i++) {
            h = hash_insert(h, i, (void*)(intptr_t)i);
        }
        kill(h);
    }
    return scale;
}

static _Bool is_ctx(void *ptr, void *ctx) { return ptr == ctx; }

static double lookup(int loops) {
    while (loops --> 0) {
        expect(hash_lookup(h, 42, is_ctx, (void*)42));
    }
    return 1;
}

int main(int argc, char **argv) {
    double const goal_ns = argc > 1 ? atof(argv[1]) : 1e6;
    /* global */ scale   = argc > 2 ? atoi(argv[2]) : 1024;

    {
        printf("growth\t%.3gns\n", bench(goal_ns, growth));
        kill(h);
    }

    {
        for (int i = 0; i < scale; i++) {
            h = hash_insert(h, i, (void*)(intptr_t)i);
        }
        printf("lookup\t%.3gns\n", bench(goal_ns, lookup));
        kill(h);
    }

    return 0;
}
