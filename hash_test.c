#include "hash.h"
#include "test.h"

static _Bool is_val(int val, void *ctx) {
    return val == *(int*)ctx;
}

static void test_basics(void) {
    struct hash *h = NULL;
    int val;

    val = 6; expect(!hash_lookup(h, 5, is_val, &val));
    val = 8; expect(!hash_lookup(h, 7, is_val, &val));

    h = hash_insert(h, 5, 6);

    val = 6; expect( hash_lookup(h, 5, is_val, &val));
    val = 8; expect(!hash_lookup(h, 5, is_val, &val));
    val = 8; expect(!hash_lookup(h, 7, is_val, &val));

    h = hash_insert(h, 7, 8);

    val = 6; expect( hash_lookup(h, 5, is_val, &val));
    val = 8; expect( hash_lookup(h, 7, is_val, &val));
    val = 6; expect(!hash_lookup(h, 7, is_val, &val));
    val = 8; expect(!hash_lookup(h, 5, is_val, &val));

    kill(h);
}

static void test_thorough(void) {
    struct hash *h = NULL;
    int const K = 47;

    for (int i = 0; i < K; i++) {
        for (int j = 0; j < K; j++) {
            expect(hash_lookup(h, j, is_val, &j) == (j<i));
        }
        h = hash_insert(h, i, i);
    }
    kill(h);
}

static void test_zero_val(void) {
    struct hash *h = NULL;
    int val = 0;

    expect(!hash_lookup(h, 42, is_val, &val));
    h = hash_insert(h, 42, val);
    expect( hash_lookup(h, 42, is_val, &val));

    kill(h);
}

static void test_zero_hash(void) {
    struct hash *h = NULL;
    int val = 42;

    expect(!hash_lookup(h, 0, is_val, &val));
    h = hash_insert(h, 0, val);
    expect( hash_lookup(h, 0, is_val, &val));

    kill(h);
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

    test_basics();
    test_thorough();
    test_zero_val();
    test_zero_hash();

    bench(growth);
    bench(lookup);

    return 0;
}
