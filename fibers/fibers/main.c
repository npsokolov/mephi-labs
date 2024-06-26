#include "fiber.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

const int64_t kArgA = 0xAAA;
const int64_t kArgB = 0xBBB;

int FiberIds[64];
int CurrentId = 0;

void FuncA(void* data) {
    assert(data == (void*)kArgA);

    FiberIds[CurrentId++] = 2;

    printf("FuncA: data=%p\n", data);
}

void FuncB(void* data) {
    assert(data == (void*)kArgB);

    FiberIds[CurrentId++] = 1;

    for (int i = 0; i < 3; ++i) {
        printf("FuncB: i=%d, data=%p\n", i, data);

        FiberSpawn(FuncA, (void*)kArgA);
        FiberYield();

        FiberIds[CurrentId++] = 1;
    }
}

void Test() {
    printf("main\n");
    FiberSpawn(FuncB, (void*)kArgB);

    FiberIds[CurrentId++] = 0;
    while (!FiberTryJoin()) {
        printf("main\n");
        FiberYield();
        FiberIds[CurrentId++] = 0;
    }

    assert(CurrentId == 12);

    for (int i = 0; i < CurrentId; ++i) {
        const int r = i % 3;
        if (r == 0 && i == 0) {
            assert(FiberIds[i] == 0);
            continue;
        }
        assert(FiberIds[i] == 2 - r);
    }
}

int main() {
    Test();

    return 0;
}
