#pragma once

#include <stddef.h>

struct vm_builder* vm_builder(void);
struct vm_program* vm_compile(struct vm_builder*);
size_t             vm_scratch(struct vm_program const*);
void               vm_run_   (struct vm_program const*, void *scratch, int n, void *ptr[]);
#define vm_run(p, scratch, n, uniforms, ...) vm_run_(p,scratch,n, (void*[]){uniforms, __VA_ARGS__})

int  vm_splat  (struct vm_builder*, unsigned bits);
int  vm_uniform(struct vm_builder*, unsigned off);
int  vm_load   (struct vm_builder*, unsigned ptr);
void vm_store  (struct vm_builder*, unsigned ptr, int x);

int vm_add(struct vm_builder*, int,int);
int vm_sub(struct vm_builder*, int,int);
int vm_mul(struct vm_builder*, int,int);
int vm_div(struct vm_builder*, int,int);
int vm_mad(struct vm_builder*, int,int,int);

int vm_eq(struct vm_builder*, int,int);
int vm_ne(struct vm_builder*, int,int);
int vm_lt(struct vm_builder*, int,int);
int vm_le(struct vm_builder*, int,int);
int vm_gt(struct vm_builder*, int,int);
int vm_ge(struct vm_builder*, int,int);

int vm_not(struct vm_builder*, int);
int vm_and(struct vm_builder*, int,int);
int vm_or (struct vm_builder*, int,int);
int vm_xor(struct vm_builder*, int,int);
int vm_sel(struct vm_builder*, int,int,int);
