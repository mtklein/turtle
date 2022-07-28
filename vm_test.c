#include "test.h"
#include "vm.h"
#include <stdlib.h>

#define len(arr) (int)(sizeof arr / sizeof *arr)

static _Bool equiv(float x, float y) {
    return (x <= y && y <= x)
        || (x != x && y != y);
}

int main(void) {
    struct builder *b = vm_builder();

    int x = vm_load(b, 1),
        y = vm_load(b, 2);
    vm_store(b, 1, vm_add(b, x,y));

    struct program *p = vm_compile(b);
    expect(p);
    b = NULL;

    char scratch[(2+2+4+3+1)*16];
    expect(vm_scratch(p) <= sizeof scratch);

    float xs[] = {0,1,2,3,4,5,6,7,8},
          ys[] = {1,2,3,4,5,6,7,8,9};

    vm_run(p,NULL,len(xs),NULL,xs,ys);
    for (int i = 0; i < len(xs); i++) {
        expect(equiv(xs[i], 2*(float)i+1));
    }

    vm_run(p,scratch,len(xs),NULL,xs,ys);
    for (int i = 0; i < len(xs); i++) {
        expect(equiv(xs[i], 3*(float)i+2));
    }

    free(p);
    return 0;
}
