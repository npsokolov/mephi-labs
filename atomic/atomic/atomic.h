#pragma once

#include <stdint.h>

inline void AtomicAdd(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock addq %1, %0\n\t"
    : "+m" (*atomic)
    : "r"(value)
    :
    );
}

inline void AtomicSub(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock subq %1, %0\n\t"
    : "+m" (*atomic)
    : "r"(value)
    :
    );
}

inline int64_t AtomicXchg(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock xchg %1, %0\n\t"
    : "+m" (*atomic), "+r"(value)
    : 
    :
    );
    return value;
}

inline int64_t AtomicCas(int64_t* atomic, int64_t* expected, int64_t value) {
    int64_t ans = 0;
    asm volatile (
        "lock cmpxchgq %3, %1\n\t"
        "setz %b0\n\t"
    : "+r" (ans), "+m" (*atomic), "+a" (*expected)
    : "r" (value)
    : "memory"
    );
    return ans;
}
