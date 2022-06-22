#include "bit_ceil.h"
#include "expect.h"
#include <limits.h>

int main(void) {
    expect(bit_ceil(0) == 0);
    expect(bit_ceil(1) == 1);
    expect(bit_ceil(2) == 2);
    expect(bit_ceil(3) == 4);
    for (size_t lg = 2; lg < CHAR_BIT * sizeof lg; lg++) {
        size_t pow2 = (size_t)1 << lg;
        expect(bit_ceil(pow2-1) == pow2);
    }
    return 0;
}
