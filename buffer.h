#pragma once

#include <stddef.h>

void* buffer_push_(void*, size_t, size_t);

#define buffer_push(ptr, n) buffer_push_(ptr, (size_t)(n), sizeof *(ptr))
