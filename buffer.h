#pragma once

#include <stddef.h>

/* buffer_push() grows a buffer's capacity to accommodate its (externally tracked) length:

    T *buf = NULL;
    int len = 0;
    for (...) {
        buf = buffer_push(buf, len);
        buf[len++] = ...;
    }
    free(buf);
*/

#define buffer_push(buf, len) buffer_push_(buf, (size_t)(len), sizeof *(buf))
void* buffer_push_(void*, size_t, size_t);
