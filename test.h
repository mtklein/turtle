#pragma once

#include <stdio.h>
#include <stdlib.h>

// kill() is a slightly safer free().
#define kill(ptr) free(ptr), ptr=NULL

// expect() is assert() but never disabled and with no call stack to wade through.
#define expect(x) \
    if (!(x)) fprintf(stderr, "%s:%d expect(%s) failed\n", __FILE__, __LINE__, #x), __builtin_trap()

// bench(fn) calls fn() with increasing loops until fn() takes at least bench_goal_ns, then
// prints the time per iteration divided through by any additional scale factor returned by fn().
extern double bench_goal_ns;
double bench_(double (*fn)(int loops));
#define bench(fn) fprintf(stdout, "%s\t%.2g\n", #fn, bench_(fn))
