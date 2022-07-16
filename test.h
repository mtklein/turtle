#pragma once

// expect() is assert() but never disabled and with no call stack to wade through.
#define expect(x) if (!(x)) expect_fail(__FILE__,__LINE__,#x), __builtin_trap()
void expect_fail(char const*, int, char const*);

// bench() calls fn() with increasing loops until fn() takes at least bench_goal_ns, then
// prints the time per iteration divided through by any additional scale factor returned by fn().
extern double bench_goal_ns;
void bench(char const *label, double (*fn)(int loops));
