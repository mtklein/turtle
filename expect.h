#pragma once

__attribute__((noreturn))
void expect_fail(const char*, int, const char*);

#define expect(x) if (!(x)) expect_fail(__FILE__, __LINE__, #x)
