#include "backtrace.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define CHECK_PTR(from, ptr, to) \
fprintf(stderr, "checking: %s: %p <= %p < %p\n", #ptr, from, ptr, to); \
assert((void*)from <= ptr && ptr < (void*)to);

void* bt0[256];
void* bt1[256];
void* bt2[256];

void Func0();
void Func1();
void Func2();
void Func3();
void Func4();
void Func5();

void Func0() {
    Func1();
    Func2();
}

void Func1() {
    int sz = Backtrace(bt0, 256);
    assert(sz == 4);
}

void Func2() {
    int sz = Backtrace(bt1, 256);
    assert(sz == 4);
}

void Func3() {
    int sz = Backtrace(bt2, 256);
    assert(sz == 5);
}

void Func4() {
    Func3();
}

void Func5() {
    Func4();
}

void TestBacktrace() {
    Func0();
    Func5();

    CHECK_PTR(Func1, bt0[0], Func2);
    CHECK_PTR(Func0, bt0[1], Func1);

    CHECK_PTR(Func2, bt1[0], Func3);
    CHECK_PTR(Func0, bt1[1], Func1);

    CHECK_PTR(Func3, bt2[0], Func4);
    CHECK_PTR(Func4, bt2[1], Func5);
    CHECK_PTR(Func5, bt2[2], TestBacktrace);
}

void TestSymbols() {
    assert(strcmp("Func1", AddrToName(bt0[0])) == 0);
    assert(strcmp("Func0", AddrToName(bt0[1])) == 0);

    assert(strcmp("Func2", AddrToName(bt1[0])) == 0);
    assert(strcmp("Func0", AddrToName(bt1[1])) == 0);

    assert(strcmp("Func3", AddrToName(bt2[0])) == 0);
    assert(strcmp("Func4", AddrToName(bt2[1])) == 0);
    assert(strcmp("Func5", AddrToName(bt2[2])) == 0);
}

int RecA(int n, void* bt[]);
int RecB(int n, void* bt[]);

struct Lens {
    int len_a;
    int len_b;
};

struct Lens Rec(int n, void* bt_a[], void* bt_b[]) {
    struct Lens lens;
    lens.len_a = RecA(n, bt_a);
    lens.len_b = RecB(n, bt_b);
    return lens;
}

int RecA(int n, void* bt[]) {
    if (n > 1) {
        return RecA(n - 1, bt);
    } else {
        return Backtrace(bt, 256);
    }
}

int RecB(int n, void* bt[]) {
    if (n > 1) {
        return RecB(n - 1, bt);
    } else {
        return Backtrace(bt, 256);
    }
}

void TestRecursion() {
    void* bt_a[256];
    void* bt_b[256];

    int min_len = 0;

    for (int r = 1; r < 20; r += 5) {
        struct Lens lens = Rec(r, bt_a, bt_b);

        if (r == 1) {
            min_len = lens.len_a;
        }

        assert(lens.len_a == lens.len_b);
        assert(lens.len_a == min_len + r - 1);

        for (int i = 0; i < r; ++i) {
            CHECK_PTR(RecA, bt_a[i], RecB);
        }
        CHECK_PTR(Rec, bt_a[r], RecA);
        for (int i = 0; i < r; ++i) {
            CHECK_PTR(RecB, bt_b[i], TestRecursion);
        }
        CHECK_PTR(Rec, bt_b[r], RecA);
    }
}

void Print0() {
    PrintBt();
}

void Print1() {
    Print0();
}

void TestPrint() {
    Print1();
}

int main(int argc, const char* argv[]) {
    TestBacktrace();
    TestSymbols();
    TestRecursion();
    TestPrint();

    return 0;
}
