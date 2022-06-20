#include "bench.h"
#include "hash.h"
#undef NDEBUG
#include <assert.h>
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
        assert(hash_lookup(h, 42, is_ctx, (void*)42));
    }
    return 1;
}

int main(int argc, char **argv) {
    /* global */ scale   = argc > 1 ? atoi(argv[1]) : 1024;
    double const goal_ns = argc > 2 ? atof(argv[2]) : 1e6;

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
