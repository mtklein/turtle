#include "hash.h"
#include "test.h"

static void test(struct hash* (*fn)(struct hash*)) {
    struct hash *h = fn(NULL);
    kill(h);
}

static _Bool is_val(int val, void *ctx) {
    return val == *(int*)ctx;
}

static struct hash* basics(struct hash *h) {
    int val;
    val = 5; expect(!hash_lookup(h, 5, is_val, &val));
    val = 7; expect(!hash_lookup(h, 7, is_val, &val));

    h = hash_insert(h, 5, 5);

    val = 5; expect( hash_lookup(h, 5, is_val, &val));
    val = 7; expect(!hash_lookup(h, 5, is_val, &val));
    val = 7; expect(!hash_lookup(h, 7, is_val, &val));

    h = hash_insert(h, 7, 7);

    val = 5; expect( hash_lookup(h, 5, is_val, &val));
    val = 7; expect( hash_lookup(h, 7, is_val, &val));
    val = 5; expect(!hash_lookup(h, 7, is_val, &val));
    val = 7; expect(!hash_lookup(h, 5, is_val, &val));

    return h;
}

static struct hash* thorough(struct hash *h) {
    int const K = 47;
    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            expect(hash_lookup(h, j, is_val, &j) == (j<i));
        }
        h = hash_insert(h, i, i);
    }
    return h;
}

static struct hash* zero_val(struct hash *h) {
    int val = 0;
    h = hash_insert(h, 42, val);
    expect(hash_lookup(h, 42, is_val, &val));
    return h;
}

static struct hash* zero_hash(struct hash *h) {
    int val = 42;
    h = hash_insert(h, 0, val);
    expect(hash_lookup(h, 0, is_val, &val));
    return h;
}


static double growth(int loops) {
    int const scale = 1024;
    while (loops --> 0) {
        struct hash *h = NULL;
        for (int i = 0; i < scale; i++) {
            h = hash_insert(h, i, i);
        }
        kill(h);
    }
    return scale;
}

static double lookup(int loops) {
    struct hash *h = NULL;
    for (int i = 0; i < 64; i++) {
        h = hash_insert(h, i, i);
    }
    while (loops --> 0) {
        int val = 42;
        expect(hash_lookup(h, 42, is_val, &val));
    }
    kill(h);
    return 1;
}

int main(int argc, char **argv) {
    bench_goal_ns = argc > 1 ? atof(argv[1]) : bench_goal_ns;

    test(basics);
    test(thorough);
    test(zero_val);
    test(zero_hash);

    bench(growth);
    bench(lookup);

    return 0;
}
