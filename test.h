#pragma once

int dprintf(int, char const*, ...);

#define expect(x) \
    if (!(x)) dprintf(2, "%s:%d expect(%s) failed\n", __FILE__, __LINE__, #x), __builtin_trap()

extern double bench_goal_ns;
double bench_(double (*fn)(int loops));
#define bench(fn) dprintf(1, "%s\t%.2g\n", #fn, bench_(fn))
