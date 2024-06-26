#include "spinlock.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#define ITER_COUNT (1 << 16)
#define THREAD_COUNT 50

struct SpinLock lock;

int64_t first = 0;
int64_t second = 0;

void* Summer(void* arg) {
    for (int i = 0; i < ITER_COUNT; ++i) {
        SpinLock_Lock(&lock);
        assert(first == second);
        ++first;
        ++second;
        SpinLock_Unlock(&lock);
    }
    return NULL;
}

void RunThreads(void* (*fun)(void*)) {
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_create(&threads[i], NULL, fun, NULL);
    }
    for (int i = 0; i < THREAD_COUNT; ++i) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    SpinLock_Init(&lock);
    RunThreads(Summer);

    assert(first == second);
    assert(first == ITER_COUNT * THREAD_COUNT);

    return 0;
}
