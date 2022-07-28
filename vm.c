#include "buffer.h"
#include "vm.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define K 4
typedef float    __attribute__((vector_size(K * sizeof(float   )))) V;
typedef unsigned __attribute__((vector_size(K * sizeof(unsigned)))) M;

struct PInst;
typedef void Stage(struct PInst const *ip, V *sp, void *ptr[], int const end, V x, V y, V z);

#define stage(name) \
    static void name(struct PInst const *ip, V *sp, void *ptr[], int const end, V x, V y, V z)
#define next (ip[1].fn)(ip+1,sp,ptr,end, x,y,z); return

typedef struct {
    Stage   *fn;
    int      x,y,z;
    unsigned arg;
} BInst;

typedef struct PInst {
    Stage    *fn;
    uintptr_t arg;  // Really just unsigned, but might as well expand up to padding.
} PInst;

typedef struct vm_builder {
    BInst *inst;
    int    insts,padding;
} Builder;

typedef struct vm_program {
    int   insts,padding;
    PInst inst[];
} Program;

Builder* vm_builder(void) {
    Builder *b = calloc(1, sizeof *b);
    return b;
}

stage(ldx) { x = sp[ip->arg]; next; }
stage(ldy) { y = sp[ip->arg]; next; }
stage(ldz) { z = sp[ip->arg]; next; }
stage(stx) { sp[ip->arg] = x; next; }

stage(done) {
    (void)ip;
    (void)sp;
    (void)ptr;
    (void)end;
    (void)x;
    (void)y;
    (void)z;
}

Program* vm_compile(Builder *b) {
    // Each might turn into ldx, ldy, ldz, the op itself, and finally stx, plus one more for done.
    Program *p = malloc(sizeof *p + (size_t)(5 * b->insts + 1) * sizeof *p->inst);

    // Which value is in each x,y,z register?
    int x=0,y=0,z=0;

    PInst *pi = p->inst;
    for (int i = 0; i < b->insts; i++) {
        BInst const bi = b->inst[i];

        if (bi.x && bi.x != x) { *pi++ = (PInst){ldx, (unsigned)bi.x-1}; x = bi.x; }
        if (bi.y && bi.y != y) { *pi++ = (PInst){ldy, (unsigned)bi.y-1}; y = bi.y; }
        if (bi.z && bi.z != z) { *pi++ = (PInst){ldz, (unsigned)bi.z-1}; z = bi.z; }

        *pi++ = (PInst){bi.fn, bi.arg};
        x = i+1;

        *pi++ = (PInst){stx, (unsigned)i};
    }
    *pi++ = (PInst){done, 0};

    p->insts = (int)(pi - p->inst);

    free(b->inst);
    free(b);
    return p;
}

size_t vm_scratch(Program const *p) {
    return sizeof(V) * (size_t)p->insts;
}

void vm_run_(Program const *p, void *scratch, int n, void *ptr[]) {
    PInst const *ip = p->inst;
    V* sp = scratch ? scratch : malloc(vm_scratch(p));

    int i = 0;
    for (; i+K <= n; i += K) { ip->fn(ip,sp,ptr,i+K, (V){0}, (V){0}, (V){0}); }
    for (; i+1 <= n; i += 1) { ip->fn(ip,sp,ptr,i+1, (V){0}, (V){0}, (V){0}); }

    if (!scratch) { free(sp); }
}

stage(splat) {
    unsigned bits;
    memcpy(&bits, &ip->arg, sizeof bits);
    x = (V)( (M){0} ^ bits );
    next;
}
stage(uniform) {
    unsigned bits;
    memcpy(&bits, (char const*)ptr[0] + ip->arg, sizeof bits);
    x = (V)( (M){0} ^ bits );
    next;
}

stage(load) {
    (end % K) ? memcpy(&x, (float const*)ptr[ip->arg] + end - 1, 1*sizeof(float))
              : memcpy(&x, (float const*)ptr[ip->arg] + end - K, K*sizeof(float));
    next;
}
stage(store) {
    (end % K) ? memcpy((float*)ptr[ip->arg] + end - 1, &x, 1*sizeof(float))
              : memcpy((float*)ptr[ip->arg] + end - K, &x, K*sizeof(float));
    next;
}

stage(add ) { x = x+y  ; next; }
stage(sub ) { x = x-y  ; next; }
stage(mul ) { x = x*y  ; next; }
stage(div_) { x = x/y  ; next; }
stage(mad ) { x = x*y+z; next; }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
stage(eq) { x = (V)( x == y ); next; }
stage(ne) { x = (V)( x != y ); next; }
stage(lt) { x = (V)( x <  y ); next; }
stage(le) { x = (V)( x <= y ); next; }
#pragma GCC diagnostic pop

stage(not) { x = (V)(~(M)x        ); next; }
stage(and) { x = (V)( (M)x & (M)y ); next; }
stage(or)  { x = (V)( (M)x | (M)y ); next; }
stage(xor) { x = (V)( (M)x ^ (M)y ); next; }
stage(sel) {
    x = (V)( ( (M)x & (M)y) |
             (~(M)x & (M)z) );
    next;
}

#if 1
__attribute__((noinline))  // This saves a ton of code size.
#endif
static int inst(Builder *b, int x, int y, int z, unsigned arg, Stage *fn) {
    b->inst = buffer_push(b->inst, b->insts);
    b->inst[b->insts] = (BInst){fn,x,y,z,arg};
    return ++b->insts;  // IDs will be 1-indexed, leaving 0 to mark unused IDs.
}

int  vm_splat  (Builder *b, unsigned bits      ) { return inst(b, 0,0,0, bits, splat  ); }
int  vm_uniform(Builder *b, unsigned off       ) { return inst(b, 0,0,0, off , uniform); }
int  vm_load   (Builder *b, unsigned ptr       ) { return inst(b, 0,0,0, ptr , load   ); }
void vm_store  (Builder *b, unsigned ptr, int x) { (void) inst(b, x,0,0, ptr , store  ); }

int vm_add(Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, add ); }
int vm_sub(Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, sub ); }
int vm_mul(Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, mul ); }
int vm_div(Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, div_); }
int vm_mad(Builder *b, int x, int y, int z) { return inst(b, x,y,z, 0, mad ); }

int vm_eq(Builder *b, int x, int y) { return inst(b, x,y,0, 0, eq); }
int vm_ne(Builder *b, int x, int y) { return inst(b, x,y,0, 0, ne); }
int vm_lt(Builder *b, int x, int y) { return inst(b, x,y,0, 0, lt); }
int vm_le(Builder *b, int x, int y) { return inst(b, x,y,0, 0, le); }
int vm_gt(Builder *b, int x, int y) { return vm_lt(b,y,x); }
int vm_ge(Builder *b, int x, int y) { return vm_le(b,y,x); }

int vm_and(Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, and); }
int vm_or (Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, or ); }
int vm_xor(Builder *b, int x, int y       ) { return inst(b, x,y,0, 0, xor); }
int vm_sel(Builder *b, int x, int y, int z) { return inst(b, x,y,z, 0, sel); }
int vm_not(Builder *b, int x              ) { return inst(b, x,0,0, 0, not); }
