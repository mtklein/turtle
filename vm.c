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

struct builder {
    struct binst *inst;
    int           insts,padding;
};
struct program {
    int          insts,padding;
    struct pinst inst[];
};

struct builder* vm_builder(void) {
    struct builder *b = calloc(1, sizeof *b);
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

struct program* vm_compile(struct builder *b) {
    int insts = 1;  /* done */
    for (int i = 0; i < b->insts; i++) {
        // TODO: the whole point of the exercise is to elide as many loads and stores as possible.
        if (b->inst[i].x) { insts++; /*load x*/ }
        if (b->inst[i].y) { insts++; /*load y*/ }
        if (b->inst[i].z) { insts++; /*load z*/ }
        if (     1      ) { insts++; /*do it!*/  }
        if (     1      ) { insts++; /*save x*/ }
    }

    struct program *p = malloc(sizeof *p + (size_t)insts * sizeof *p->inst);
    p->insts = insts;

    struct pinst *inst = p->inst;
    for (int i = 0; i < b->insts; i++) {
        if (b->inst[i].x) { *inst++ = (struct pinst){ldx, (unsigned)b->inst[i].x-1, 0}; }
        if (b->inst[i].y) { *inst++ = (struct pinst){ldy, (unsigned)b->inst[i].y-1, 0}; }
        if (b->inst[i].z) { *inst++ = (struct pinst){ldz, (unsigned)b->inst[i].z-1, 0}; }
        *inst++ = (struct pinst){b->inst[i].fn, b->inst[i].arg, 0};
        *inst++ = (struct pinst){stx, (unsigned)i, 0};
    }
    *inst++ = (struct pinst){done, 0,0};

    free(b->inst);
    free(b);
    return p;
}

size_t vm_scratch(struct program const *p) {
    return sizeof(V) * (size_t)p->insts;
}

void vm_run_(struct program const *p, void *scratch, int n, void *ptr[]) {
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
static int inst(struct builder *b, int x, int y, int z, unsigned arg, stage *fn) {
    b->inst = buffer_push(b->inst, b->insts);
    b->inst[b->insts] = (struct binst){fn,x,y,z,arg};
    return ++b->insts;  // IDs will be 1-indexed, leaving 0 to mark unused IDs.
}

int vm_splat  (struct builder *b, unsigned bits      ) { return inst(b,0,0,0, bits,splat  ); }
int vm_uniform(struct builder *b, unsigned off       ) { return inst(b,0,0,0,  off,uniform); }
int vm_load   (struct builder *b, unsigned ptr       ) { return inst(b,0,0,0,  ptr,load   ); }
int vm_store  (struct builder *b, unsigned ptr, int x) { return inst(b,x,0,0,  ptr,store  ); }

int vm_add(struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,add ); }
int vm_sub(struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,sub ); }
int vm_mul(struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,mul ); }
int vm_div(struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,div_); }
int vm_mad(struct builder *b, int x, int y, int z) { return inst(b,x,y,z,0,mad ); }

int vm_eq(struct builder *b, int x, int y) { return inst(b,x,y,0,0,eq); }
int vm_ne(struct builder *b, int x, int y) { return inst(b,x,y,0,0,ne); }
int vm_lt(struct builder *b, int x, int y) { return inst(b,x,y,0,0,lt); }
int vm_le(struct builder *b, int x, int y) { return inst(b,x,y,0,0,le); }
int vm_gt(struct builder *b, int x, int y) { return vm_lt(b,y,x); }
int vm_ge(struct builder *b, int x, int y) { return vm_le(b,y,x); }

int vm_and(struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,and); }
int vm_or (struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,or ); }
int vm_xor(struct builder *b, int x, int y       ) { return inst(b,x,y,0,0,xor); }
int vm_sel(struct builder *b, int x, int y, int z) { return inst(b,x,y,z,0,sel); }
int vm_not(struct builder *b, int x              ) { return inst(b,x,0,0,0,not); }
