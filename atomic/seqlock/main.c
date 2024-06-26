#include "seqlock.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#define ITER_COUNT (1 << 14)
#define READ_THREAD_COUNT 1000
#define WRITE_THREAD_COUNT 50

struct SeqLock lock;

volatile int64_t first = 0;
volatile int64_t second = 0;
volatile int64_t write_count = 0;
volatile int64_t read_count = 0;

void* Reader(void* arg) {
    while (write_count < WRITE_THREAD_COUNT) {
        int64_t first_value;
        int64_t second_value;
        int64_t s;

        do {
            s = SeqLock_ReadLock(&lock);
            first_value = first;
            second_value = second;
        } while (!SeqLock_ReadUnlock(&lock, s));

        assert(first_value == second_value);
        AtomicAdd((int64_t*)&read_count, 1);
    }

    return NULL;
}

void* Writer(void* arg) {
    for (int i = 0; i < ITER_COUNT; ++i) {
        SeqLock_WriteLock(&lock);
        ++first;
        ++second;
        SeqLock_WriteUnlock(&lock);
    }

    AtomicAdd((int64_t*)&write_count, 1);
    return NULL;
}

void RunThreads(void* (*reader)(void*), void* (*writer)(void*)) {
    pthread_t read_threads[READ_THREAD_COUNT];
    pthread_t write_threads[WRITE_THREAD_COUNT];

    for (int i = 0; i < READ_THREAD_COUNT; ++i) {
        pthread_create(&read_threads[i], NULL, reader, NULL);
    }
    for (int i = 0; i < WRITE_THREAD_COUNT; ++i) {
        pthread_create(&write_threads[i], NULL, writer, NULL);
    }

    for (int i = 0; i < READ_THREAD_COUNT; ++i) {
        pthread_join(read_threads[i], NULL);
    }
    for (int i = 0; i < WRITE_THREAD_COUNT; ++i) {
        pthread_join(write_threads[i], NULL);
    }
}

int main() {
    SeqLock_Init(&lock);

    RunThreads(Reader, Writer);

    assert(read_count > 50);

    return 0;
}
