#undef UNITY_BUILD
#include "hash.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

struct hash {
    int len, mask;
    struct {
        int  live, hash;
        void *val;
    } table[];
};

static void hash_just_insert(struct hash *h, int hash, void *val) {
    assert(h && h->len <= h->mask);
    int const mask = h->mask;

    for (int i = hash & mask, round = 0; round <= mask; round++) {
        if (h->table[i].live == 0) {
            h->table[i].live = 1;
            h->table[i].hash = hash;
            h->table[i].val  = val;
            h->len++;
            return;
        }
        i = (i+1) & mask;
    }

    assert(0 && "unreachable");
}

struct hash* hash_insert(struct hash *h, int hash, void *val) {
    int const len = h ? h->len    : 0,
              cap = h ? h->mask+1 : 0;

    if (len >= cap*3/4) {
        int const new_cap = cap ? 2*cap : 1;

        struct hash *grown = calloc(1, sizeof *h + (size_t)new_cap * sizeof *h->table);
        grown->mask = new_cap-1;

        for (int i = 0; i < cap; i++) {
            if (h->table[i].live) {
                hash_just_insert(grown, h->table[i].hash, h->table[i].val);
            }
        }
        assert(grown->len == len);

        free(h);
        h = grown;
    }

    hash_just_insert(h, hash, val);
    return h;
}

bool hash_lookup(struct hash const *h, int hash, bool(*match)(void *val, void *ctx), void *ctx) {
    int const mask = h ? h->mask : -1;

    for (int i = hash & mask, round = 0; round <= mask; round++) {
        if (h->table[i].live == 0) {
            return false;
        }
        if (h->table[i].hash == hash && match(h->table[i].val, ctx)) {
            return true;
        }
        i = (i+1) & mask;
    }
    return false;
}
