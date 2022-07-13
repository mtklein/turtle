#pragma once

int dprintf(int, char const*, ...);

#define expect(x) \
    if (!(x)) dprintf(2, "%s:%d expect(%s) failed\n", __FILE__, __LINE__, #x), __builtin_trap()

double bench_(double goal_ns, double (*fn)(int loops));
#define bench(goal_ns, fn) dprintf(1, "%s\t%.2g\n", #fn, bench_(goal_ns, fn))
