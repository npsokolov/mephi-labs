#include "atomic.h"

#include <assert.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#define ITER_COUNT (1 << 16)
#define THREAD_COUNT 1000

void* Call(void (*fun)(int64_t*, int64_t), void* arg1, int64_t arg2) {
    for (int i = 0; i < ITER_COUNT; ++i) {
        fun((int64_t*)arg1, arg2);
    }
    return NULL;
}

void* Add1Func(void* arg) {
    return Call(AtomicAdd, arg, 1);
}

void* Add2Func(void* arg) {
    return Call(AtomicAdd, arg, 2);
}

void* Sub1Func(void* arg) {
    return Call(AtomicSub, arg, 1);
}

void* Sub2Func(void* arg) {
    return Call(AtomicSub, arg, 2);
}

struct XchgOpts {
    int64_t flag;
    int64_t counter;
};

void* XchgFunc(void* arg) {
    struct XchgOpts* x = (struct XchgOpts*)arg;

    while (AtomicXchg(&x->flag, 0) == 0)
        ;

    ++x->counter;

    asm volatile ("movq $1, %0" : "=m"(x->flag));

    return NULL;
}

void* CasFunc(void* arg) {
    int64_t* atomic = (int64_t*)arg;
    int64_t expected = *atomic;
    int64_t value;

    do {
        value = expected + 1 + 2 * (expected & 1);
    } while (AtomicCas(atomic, &expected, value) == 0);

    return NULL;
}

void RunThreads(void* (*fun)(void*), void* arg) {
    pthread_t threads[THREAD_COUNT];
    for (int i = 0; i < THREAD_COUNT; ++i){
        pthread_create(&threads[i], NULL, fun, arg);
    }
    for (int i = 0; i < THREAD_COUNT; ++i){
        pthread_join(threads[i], NULL);
    }
}

void TestAddSub() {
    int64_t value = 0;

    RunThreads(Add1Func, &value);
    assert(value == ITER_COUNT * THREAD_COUNT);

    RunThreads(Sub1Func, &value);
    assert(value == 0);

    RunThreads(Add2Func, &value);
    assert(value == 2 * ITER_COUNT * THREAD_COUNT);

    RunThreads(Sub2Func, &value);
    assert(value == 0);
}

void TestXchg() {
    for (int i = 0; i < 200; ++i) {
        struct XchgOpts x = {1, 0};
        RunThreads(XchgFunc, &x);
        assert(x.counter == THREAD_COUNT);
    }
}

void TestCas() {
    for (int i = 0; i < 200; ++i) {
        int64_t x = 1;
        RunThreads(CasFunc, &x);
        assert(x == 2 * THREAD_COUNT + 1);
    }
}

int main() {
    TestAddSub();
    TestXchg();
    TestCas();

    return 0;
}
