#include "hash.h"
#include "test.h"
#include <stdint.h>
#include <stdlib.h>

#define kill(ptr) free(ptr), ptr=NULL

static struct hash *h;
static int         scale;

static double growth(int loops) {
    while (loops --> 0) {
        for (int i = 0; i < scale; i++) {
            h = hash_insert(h, i, i);
        }
        kill(h);
    }
    return scale;
}

static _Bool is_ctx(int val, void *ctx) {
    return val == (intptr_t)ctx;
}

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
        bench(goal_ns, growth);
        kill(h);
    }

    {
        for (int i = 0; i < scale; i++) {
            h = hash_insert(h, i, i);
        }
        bench(goal_ns, lookup);
        kill(h);
    }

    return 0;
}
