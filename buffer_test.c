#include "buffer.h"
#include "test.h"
#include <stdlib.h>

int main(void) {
    const int k = 20000;

    int *b = NULL;
    for (int i = 0; i < k; i++) {
        b = buffer_push(b,i);
        b[i] = i;
    }
    for (int i = 0; i < k; i++) {
        expect(b[i] == i);
    }
    free(b);

    return 0;
}
