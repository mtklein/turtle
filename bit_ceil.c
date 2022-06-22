#include "bit_ceil.h"
#include <limits.h>

size_t bit_ceil(size_t x) {
    if (x <= 1) {
        return x;
    }
    return (size_t)1 << (CHAR_BIT*sizeof(x) - (size_t)__builtin_clzll((unsigned long long)x-1));
}
