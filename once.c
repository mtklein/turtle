#include "once.h"
#include <stdatomic.h>

enum { INIT, BUSY, DONE };

void once(char _Atomic *flag, void (*fn)(void *ctx), void *ctx) {
    char st = atomic_load_explicit(flag, memory_order_acquire);

    if (st == DONE) {
        return;
    }

    if (st == INIT && atomic_compare_exchange_strong_explicit(flag, &st, BUSY,
                                                              memory_order_relaxed,
                                                              memory_order_relaxed)) {
        fn(ctx);
        atomic_store_explicit(flag, DONE, memory_order_release);
        return;
    }

    while (atomic_load_explicit(flag, memory_order_acquire) != DONE);
}
