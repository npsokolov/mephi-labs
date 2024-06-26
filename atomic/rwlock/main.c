#include "rwlock.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#define READER_THREAD_COUNT 50
#define WRITER_THREAD_COUNT 10

struct RwLock lock;

volatile int64_t first = 0;
volatile int64_t second = 0;
volatile int64_t write_count = 0;
volatile int64_t read_count = 0;

void* Reader(void* arg) {
    while (write_count < WRITER_THREAD_COUNT) {
        RwLock_ReadLock(&lock);
        assert(first == second);
        AtomicAdd((int64_t*)&read_count, 1);
        RwLock_ReadUnlock(&lock);
    }
    return NULL;
}

void* Writer(void* arg) {
    for (int i = 0; i < (1 << 10); ++i) {
        RwLock_WriteLock(&lock);
        ++first;
        ++second;
        RwLock_WriteUnlock(&lock);
    }

    AtomicAdd((int64_t*)&write_count, 1);

    return 0;
}

void RunThreads(void* (*reader)(void*), void* (*writer)(void*)) {
    pthread_t reader_threads[READER_THREAD_COUNT];
    pthread_t writer_threads[WRITER_THREAD_COUNT];

    for (int i = 0; i < READER_THREAD_COUNT; ++i) {
        pthread_create(&reader_threads[i], NULL, reader, NULL);
    }
    for (int i = 0; i < WRITER_THREAD_COUNT; ++i) {
        pthread_create(&writer_threads[i], NULL, writer, NULL);
    }

    for (int i = 0; i < READER_THREAD_COUNT; ++i) {
        pthread_join(reader_threads[i], NULL);
    }
    for (int i = 0; i < WRITER_THREAD_COUNT; ++i) {
        pthread_join(writer_threads[i], NULL);
    }
}

int main() {
    RwLock_Init(&lock);
    RunThreads(Reader, Writer);

    assert(read_count > 50);

    return 0;
}
