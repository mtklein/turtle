#pragma once

// Create with hash_insert(NULL, ...); clean up with free().
struct hash* hash_insert(struct hash       *, int hash, void *val);
_Bool        hash_lookup(struct hash const *, int hash, _Bool(*)(void *val, void *ctx), void *ctx);

#if defined(UNITY_BUILD)
    #include "hash.c"
    #define UNITY_BUILD 1
#endif
