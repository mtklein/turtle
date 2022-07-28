#pragma once

#include <stddef.h>

struct builder* vm_builder(void);
struct program* vm_compile(struct builder*);
size_t          vm_scratch(struct program const*);
void            vm_run_   (struct program const*, void *scratch, int n, void *ptr[]);
#define vm_run(p, scratch, n, uniforms, ...) vm_run_(p,scratch,n, (void*[]){uniforms, __VA_ARGS__})

int  vm_splat  (struct builder*, unsigned bits);
int  vm_uniform(struct builder*, unsigned off);
int  vm_load   (struct builder*, unsigned ptr);
void vm_store  (struct builder*, unsigned ptr, int x);

int vm_add(struct builder*, int,int);
int vm_sub(struct builder*, int,int);
int vm_mul(struct builder*, int,int);
int vm_div(struct builder*, int,int);
int vm_mad(struct builder*, int,int,int);

int vm_eq(struct builder*, int,int);
int vm_ne(struct builder*, int,int);
int vm_lt(struct builder*, int,int);
int vm_le(struct builder*, int,int);
int vm_gt(struct builder*, int,int);
int vm_ge(struct builder*, int,int);

int vm_not(struct builder*, int);
int vm_and(struct builder*, int,int);
int vm_or (struct builder*, int,int);
int vm_xor(struct builder*, int,int);
int vm_sel(struct builder*, int,int,int);
