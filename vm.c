#include "buffer.h"
#include "vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(__linux__)
    #define _GNU_SOURCE
#endif
#include <dlfcn.h>

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
    int      uses,death;
    enum Kind { MATH, SPLAT, UNIFORM, LOAD, STORE } kind;
    int      padding;
} BInst;

typedef struct PInst {
    Stage    *fn;
    unsigned  arg,padding;
} PInst;

typedef struct vm_builder {
    BInst *inst;
    int    insts,padding;
} Builder;

typedef struct vm_program {
    int   insts,padding;
    PInst inst[];
} Program;

void vm_dump_builder(FILE *f, struct vm_builder const *b);
void vm_dump_builder(FILE *f, struct vm_builder const *b) {
    fprintf(f, "vm_builder, %d insts\n", b->insts);
    for (int i = 0; i < b->insts; i++) {
        BInst bi = b->inst[i];

        fprintf(f, "%d\t(%d %d)\t", i+1, bi.uses, bi.death);
        Dl_info info;
        if (dladdr((void const*)bi.fn, &info)) {
            fprintf(f, "%s\t", info.dli_sname);
        } else {
            fprintf(f, "%p\t", (void const*)bi.fn);
        }

        if (bi.x) { fprintf(f, " x=%d", bi.x); }
        if (bi.y) { fprintf(f, " y=%d", bi.y); }
        if (bi.z) { fprintf(f, " z=%d", bi.z); }
        if ( 1  ) { fprintf(f, " arg=%x", bi.arg); }
        fprintf(f, "\n");
    }
}

void vm_dump_program(FILE *f, struct vm_program const *p);
void vm_dump_program(FILE *f, struct vm_program const *p) {
    fprintf(f, "vm_program, %d insts\n", p->insts);
    for (int i = 0; i < p->insts; i++) {
        PInst pi = p->inst[i];
        Dl_info info;
        fprintf(f, "%d\t", i);
        if (dladdr((void const*)pi.fn, &info)) {
            fprintf(f, "%s\t", info.dli_sname);
        } else {
            fprintf(f, "%p\t", (void const*)pi.fn);
        }
        fprintf(f, "%x\n", pi.arg);
    }
}

Builder* vm_builder(void) {
    Builder *b = calloc(1, sizeof *b);
    return b;
}

stage(ldx) { x = sp[ip->arg]; next; }
stage(ldy) { y = sp[ip->arg]; next; }
stage(ldz) { z = sp[ip->arg]; next; }
stage(stx) { sp[ip->arg] = x; next; }

stage(x2y) { y = x; next; }
stage(x2z) { z = x; next; }

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

    // Where does each value live?
    enum { NONE, X,Y,Z, MEMORY } *home = calloc(sizeof *home, (size_t)b->insts);

    PInst *pi = p->inst;
    for (int i = 0; i < b->insts; i++) {
        BInst const bi = b->inst[i];

        if (bi.x && bi.x != x) { *pi++ = (PInst){ldx, (unsigned)bi.x-1, 0}; x = bi.x; }
        if (bi.y && bi.y != y) { *pi++ = (PInst){ldy, (unsigned)bi.y-1, 0}; y = bi.y; }
        if (bi.z && bi.z != z) { *pi++ = (PInst){ldz, (unsigned)bi.z-1, 0}; z = bi.z; }

        *pi++ = (PInst){bi.fn, bi.arg, 0};

        // Stores produce no value.
        if (bi.kind == STORE)  {
            continue;
        }

        // The value from bi.fn is now in the x register by convention.
        x = i+1;

        // If consumed by the next instruction...
        if (bi.uses == 1 && bi.death == x+1) {
            if (b->inst[i+1].x == x) { /* it wants it in x, all good */ }
            if (b->inst[i+1].y == x) { *pi++ = (PInst){x2y,0,0}; y=x; }
            if (b->inst[i+1].z == x) { *pi++ = (PInst){x2z,0,0}; z=x; }
            continue;
        }

        *pi++ = (PInst){stx, (unsigned)x-1, 0};
    }
    *pi++ = (PInst){done, 0, 0};

    p->insts = (int)(pi - p->inst);

    free(home);
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
__attribute__((noinline))  // This saves a ton of code size.
#endif
static int inst(Builder *b, int x, int y, int z, unsigned arg, Stage *fn, enum Kind kind) {
    b->inst = buffer_push(b->inst, b->insts);
    b->inst[b->insts] = (BInst){fn,x,y,z,arg, 0,0, kind,0};
    int const id = ++b->insts;  // IDs will be 1-indexed, leaving 0 to mark unused IDs.
    if (x) { b->inst[x-1].uses++;  b->inst[x-1].death = id; }
    if (y) { b->inst[y-1].uses++;  b->inst[y-1].death = id; }
    if (z) { b->inst[z-1].uses++;  b->inst[z-1].death = id; }
    return id;
}

int  vm_splat  (Builder *b, unsigned bits      ) { return inst(b,0,0,0,bits, splat  , SPLAT  ); }
int  vm_uniform(Builder *b, unsigned off       ) { return inst(b,0,0,0,off , uniform, UNIFORM); }
int  vm_load   (Builder *b, unsigned ptr       ) { return inst(b,0,0,0,ptr , load   , LOAD   ); }
void vm_store  (Builder *b, unsigned ptr, int x) { (void) inst(b,x,0,0,ptr , store  , STORE  ); }

int vm_add(Builder *b, int x, int y       ) { return inst(b,x,y,0,0, add , MATH); }
int vm_sub(Builder *b, int x, int y       ) { return inst(b,x,y,0,0, sub , MATH); }
int vm_mul(Builder *b, int x, int y       ) { return inst(b,x,y,0,0, mul , MATH); }
int vm_div(Builder *b, int x, int y       ) { return inst(b,x,y,0,0, div_, MATH); }
int vm_mad(Builder *b, int x, int y, int z) { return inst(b,x,y,z,0, mad , MATH); }

int vm_eq(Builder *b, int x, int y) { return inst(b,x,y,0,0, eq, MATH); }
int vm_ne(Builder *b, int x, int y) { return inst(b,x,y,0,0, ne, MATH); }
int vm_lt(Builder *b, int x, int y) { return inst(b,x,y,0,0, lt, MATH); }
int vm_le(Builder *b, int x, int y) { return inst(b,x,y,0,0, le, MATH); }
int vm_gt(Builder *b, int x, int y) { return vm_lt(b,y,x); }
int vm_ge(Builder *b, int x, int y) { return vm_le(b,y,x); }

int vm_and(Builder *b, int x, int y       ) { return inst(b,x,y,0,0, and, MATH); }
int vm_or (Builder *b, int x, int y       ) { return inst(b,x,y,0,0, or , MATH); }
int vm_xor(Builder *b, int x, int y       ) { return inst(b,x,y,0,0, xor, MATH); }
int vm_sel(Builder *b, int x, int y, int z) { return inst(b,x,y,z,0, sel, MATH); }
int vm_not(Builder *b, int x              ) { return inst(b,x,0,0,0, not, MATH); }
