#include "fiber.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>

void Print(void* data) {
    for (int i = 0; i < 10000; ++i) {
        printf("i=%d, data=%p\n", i, data);
    }
}

void TestPrint() {
    FiberSpawn(Print, (void*)0x111);
    FiberSpawn(Print, (void*)0x222);
    while (!FiberTryJoin()) {
        FiberYield();
    }
}

#define SUM_ITERS 10000000

void Sum(void* data) {
    volatile int64_t* r = (volatile int64_t*)data;
    for (int i = 0; i < SUM_ITERS; ++i) {
        *r += 1;
    }
}

void TestSum() {
    int64_t r = 0;
    int n = 4;

    for (int i = 0; i < n; ++i) {
        FiberSpawn(Sum, (void*)&r);
    }
    while (!FiberTryJoin()) {
        FiberYield();
    }

    assert(SUM_ITERS <= r && r < n * SUM_ITERS);
}

struct Info {
    int id;
    int64_t* value;
};

void Checker(void* data) {
    volatile struct Info* info = (volatile struct Info*)data;
    *info->value = info->id;

    for (int i = 0; i < 2; ++i) {
        int value;
        while ((value = *info->value) == info->id)
            ;
        *info->value = info->id;
    }
}

void TestOverwrite() {
    int64_t value = 0;
    struct Info info1 = {1, &value};
    struct Info info2 = {2, &value};

    FiberSpawn(Checker, (void*)&info1);
    FiberSpawn(Checker, (void*)&info2);

    while (!FiberTryJoin()) {
        FiberYield();
    }
}

int main() {
    FiberInit();

    TestPrint();
    TestSum();
    TestOverwrite();

    return 0;
}
