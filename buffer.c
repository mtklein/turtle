#include "buffer.h"
#include <stdlib.h>

void* buffer_push_(void *buf, size_t len, size_t sizeof_T) {
    return len == 0      ? realloc(buf,       sizeof_T)
        :  len & (len-1) ? buf
        :                  realloc(buf, 2*len*sizeof_T);
}
