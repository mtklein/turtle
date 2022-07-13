#include "hash.h"
#include "test.h"
#include <stdint.h>

static void test(struct hash* (*fn)(struct hash*)) {
    struct hash *h = fn(NULL);
    kill(h);
}

static _Bool is_ctx(int val, void *ctx) {
    return val == (intptr_t)ctx;
}

static struct hash* basics(struct hash *h) {
    expect(!hash_lookup(h, 5, is_ctx, (void*)5));
    expect(!hash_lookup(h, 7, is_ctx, (void*)7));

    h = hash_insert(h, 5, 5);

    expect( hash_lookup(h, 5, is_ctx, (void*)5));
    expect(!hash_lookup(h, 5, is_ctx, (void*)7));
    expect(!hash_lookup(h, 7, is_ctx, (void*)7));

    h = hash_insert(h, 7, 7);

    expect( hash_lookup(h, 5, is_ctx, (void*)5));
    expect( hash_lookup(h, 7, is_ctx, (void*)7));
    expect(!hash_lookup(h, 5, is_ctx, (void*)7));
    expect(!hash_lookup(h, 7, is_ctx, (void*)5));

    return h;
}

static struct hash* thorough(struct hash *h) {
    int const K = 47;
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            expect(hash_lookup(h, j, is_ctx, (void*)(intptr_t)j) == (j<i));
        }
        h = hash_insert(h, i, i);
    }
    return h;
}

static struct hash* zero_val(struct hash *h) {
    h = hash_insert(h, 42, 0);
    expect(hash_lookup(h, 42, is_ctx, NULL));
    return h;
}

static struct hash* zero_hash(struct hash *h) {
    h = hash_insert(h, 0, 42);
    expect(hash_lookup(h, 0, is_ctx, (void*)(intptr_t)42));
    return h;
}

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

static double lookup(int loops) {
    while (loops --> 0) {
        expect(hash_lookup(h, 42, is_ctx, (void*)42));
    }
    return 1;
}

int main(int argc, char **argv) {
    bench_goal_ns = argc > 1 ? atof(argv[1]) : bench_goal_ns;
    scale         = argc > 2 ? atoi(argv[2]) : 1024;

    test(basics);
    test(thorough);
    test(zero_val);
    test(zero_hash);

    bench(growth);
    kill(h);

    for (int i = 0; i < scale; i++) {
        h = hash_insert(h, i, i);
    }
    bench(lookup);
    kill(h);

    return 0;
}
