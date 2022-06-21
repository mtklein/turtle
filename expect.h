#pragma once

int  dprintf(int, const char*, ...);
void abort(void);

#define expect(x) if (!(x)) dprintf(2, "%s:%d\texpect(%s) failed\n",__FILE__,__LINE__,#x),abort()
