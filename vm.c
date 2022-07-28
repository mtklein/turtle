#include "buffer.h"
#include "vm.h"
#include <stdlib.h>
#include <string.h>

#define K 4
typedef float    __attribute__((vector_size(K * sizeof(float   )))) V;
typedef unsigned __attribute__((vector_size(K * sizeof(unsigned)))) M;

struct pinst;
typedef void stage(struct pinst const *ip, V *sp, void *ptr[], int const end, V x, V y, V z);

#define stage(name) \
    static void name(struct pinst const *ip, V *sp, void *ptr[], int const end, V x, V y, V z)
#define next (ip[1].fn)(ip+1,sp,ptr,end, x,y,z); return

struct binst {
    stage   *fn;
    int      x,y,z;
    unsigned arg;
};
struct pinst {
    stage   *fn;
    unsigned arg,padding;
};

typedef struct vm_builder {
    struct binst *inst;
    int           insts,padding;
} builder;

typedef struct vm_program {
    int          insts,padding;
    struct pinst inst[];
} program;

builder* vm_builder(void) {
    builder *b = calloc(1, sizeof *b);
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

program* vm_compile(builder *b) {
    // Each might turn into ldx, ldy, ldz, the op itself, and finally stx, plus one more for done.
    program *p = malloc(sizeof *p + (size_t)(5 * b->insts + 1) * sizeof *p->inst);

    // Which value is in each x,y,z register?
    int x=0,y=0,z=0;

    struct pinst *inst = p->inst;
    for (int i = 0; i < b->insts; i++) {
        if (b->inst[i].x && b->inst[i].x != x) {
            *inst++ = (struct pinst){ldx, (unsigned)b->inst[i].x-1, 0};
            x = b->inst[i].x;
        }

        if (b->inst[i].y && b->inst[i].y != y) {
            *inst++ = (struct pinst){ldy, (unsigned)b->inst[i].y-1, 0};
            y = b->inst[i].y;
        }

        if (b->inst[i].z && b->inst[i].z != z) {
            *inst++ = (struct pinst){ldz, (unsigned)b->inst[i].z-1, 0};
            z = b->inst[i].z;
        }

        *inst++ = (struct pinst){b->inst[i].fn, b->inst[i].arg, 0};
        x = i+1;

        *inst++ = (struct pinst){stx, (unsigned)i, 0};
    }
    *inst++ = (struct pinst){done, 0,0};

    p->insts = (int)(inst - p->inst);

    free(b->inst);
    free(b);
    return p;
}

size_t vm_scratch(program const *p) {
    return sizeof(V) * (size_t)p->insts;
}

void vm_run_(program const *p, void *scratch, int n, void *ptr[]) {
    struct pinst const *ip = p->inst;
    V* sp = scratch ? scratch : malloc(vm_scratch(p));

    int i = 0;
    for (; i+K <= n; i += K) { ip->fn(ip,sp,ptr,i+K, (V){0}, (V){0}, (V){0}); }
    for (; i+1 <= n; i += 1) { ip->fn(ip,sp,ptr,i+1, (V){0}, (V){0}, (V){0}); }

    if (!scratch) { free(sp); }
}

stage(splat) {
    x = (V)( (M){0} ^ ip->arg );
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
__attribute__((noinline))
#endif
static int inst(builder *b, int x, int y, int z, unsigned arg, stage *fn) {
    b->inst = buffer_push(b->inst, b->insts);
    b->inst[b->insts] = (struct binst){fn,x,y,z,arg};
    return ++b->insts;  // IDs will be 1-indexed, leaving 0 to mark unused IDs.
}

int  vm_splat  (builder *b, unsigned bits      ) { return inst(b,0,0,0, bits, splat  ); }
int  vm_uniform(builder *b, unsigned off       ) { return inst(b,0,0,0, off , uniform); }
int  vm_load   (builder *b, unsigned ptr       ) { return inst(b,0,0,0, ptr , load   ); }
void vm_store  (builder *b, unsigned ptr, int x) { (void) inst(b,x,0,0, ptr , store  ); }

int vm_add(builder *b, int x, int y       ) { return inst(b,x,y,0,0,add ); }
int vm_sub(builder *b, int x, int y       ) { return inst(b,x,y,0,0,sub ); }
int vm_mul(builder *b, int x, int y       ) { return inst(b,x,y,0,0,mul ); }
int vm_div(builder *b, int x, int y       ) { return inst(b,x,y,0,0,div_); }
int vm_mad(builder *b, int x, int y, int z) { return inst(b,x,y,z,0,mad ); }

int vm_eq(builder *b, int x, int y) { return inst(b,x,y,0,0,eq); }
int vm_ne(builder *b, int x, int y) { return inst(b,x,y,0,0,ne); }
int vm_lt(builder *b, int x, int y) { return inst(b,x,y,0,0,lt); }
int vm_le(builder *b, int x, int y) { return inst(b,x,y,0,0,le); }
int vm_gt(builder *b, int x, int y) { return vm_lt(b,y,x); }
int vm_ge(builder *b, int x, int y) { return vm_le(b,y,x); }

int vm_and(builder *b, int x, int y       ) { return inst(b,x,y,0,0,and); }
int vm_or (builder *b, int x, int y       ) { return inst(b,x,y,0,0,or ); }
int vm_xor(builder *b, int x, int y       ) { return inst(b,x,y,0,0,xor); }
int vm_sel(builder *b, int x, int y, int z) { return inst(b,x,y,z,0,sel); }
int vm_not(builder *b, int x              ) { return inst(b,x,0,0,0,not); }
