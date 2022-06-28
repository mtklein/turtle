#pragma once

int dprintf(int, char const*, ...);

#define expect(x) \
    if (!(x)) dprintf(2, "%s:%d expect(%s) failed\n", __FILE__, __LINE__, #x), __builtin_trap()
