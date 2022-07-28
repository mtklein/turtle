#include "test.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>

void vm_dump_builder(FILE*, struct vm_builder const*);
void vm_dump_program(FILE*, struct vm_program const*);

#define len(arr) (int)(sizeof arr / sizeof *arr)

static _Bool equiv(float x, float y) {
    return (x <= y && y <= x)
        || (x != x && y != y);
}

int main(void) {
    struct vm_builder *b = vm_builder();

    int x = vm_load(b, 1),
        y = vm_load(b, 2),
        z = vm_splat(b, 0x3f800000);
    vm_store(b, 1, vm_mad(b, x,y,z));

    vm_dump_builder(stdout, b);

    struct vm_program *p = vm_compile(b);
    expect(p);
    b = NULL;

    vm_dump_program(stdout, p);

    char scratch[(2+2+1+5+3+1)*16];
    expect(vm_scratch(p) <= sizeof scratch);

    float xs[] = {0,1,2,3,4,5,6,7,8},
          ys[] = {1,2,3,4,5,6,7,8,9};

    vm_run(p,NULL,len(xs),NULL,xs,ys);
    for (int i = 0; i < len(xs); i++) {
        expect(equiv(xs[i], (float)(i * (i+1) + 1)));
    }

    vm_run(p,scratch,len(xs),NULL,xs,ys);
    for (int i = 0; i < len(xs); i++) {
        expect(equiv(xs[i], (float)((i * (i+1) + 1) * (i+1) + 1)));
    }

    free(p);
    return 0;
}
