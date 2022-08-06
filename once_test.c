#include "once.h"
#include "test.h"
#include <pthread.h>

#define len(arr) (int)(sizeof arr / sizeof *arr)

struct foo {
    char _Atomic flag;
    char         val;
};

static void inc(void *arg) {
    char *val = arg;
    (*val)++;
}

static void* each_thread(void *arg) {
    struct foo *foo = arg;
    once(&foo->flag, inc, &foo->val);
    return NULL;
}

int main(void) {
    struct foo foo = {0};

    pthread_t th[42];
    for (int i = 0; i < len(th); i++) {
        pthread_create(th+i, NULL, each_thread, &foo);
    }
    for (int i = 0; i < len(th); i++) {
        pthread_join(th[i], NULL);
    }

    expect(foo.val == 1);
    return 0;
}
