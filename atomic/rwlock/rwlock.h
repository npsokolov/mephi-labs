#pragma once

#include <stdint.h>

inline void AtomicAdd(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock addq %1, %0\n\t"
    : "+m" (*atomic)
    : "r"(value)
    : "memory"
    );
}

inline void AtomicSub(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock subq %1, %0\n\t"
    : "+m" (*atomic)
    : "r"(value)
    : "memory"
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

inline int64_t AtomicCas(int64_t* atomic, int64_t expected, int64_t value) {
    int64_t ans = 0;
    asm volatile (
        "lock cmpxchgq %3, %1\n\t"
        "setz %b0\n\t"
    : "+r" (ans), "+m" (*atomic), "+a" (expected)
    : "r" (value)
    : "memory"
    );
    return ans;
}

struct SpinLock {
    int64_t locked;
};

inline void SpinLock_Init(struct SpinLock* lock) {
    lock->locked = 0;
}

inline void SpinLock_Lock(struct SpinLock* lock) {
    int res = 0;
    while (res==0){
        res = AtomicCas(&lock->locked, 0ull, 1ull);
    }
}

inline void SpinLock_Unlock(struct SpinLock* lock) {
    __sync_synchronize();
    lock->locked = 0;
}

struct RwLock {
    struct SpinLock writeLock;
    int64_t readers;
    int64_t writing;
};

inline void RwLock_Init(struct RwLock* lock) {
    lock->readers = 0;
    lock->writing= 0;
    SpinLock_Init(&lock->writeLock);
}

inline void RwLock_ReadLock(struct RwLock* lock) {
    int res = 0;
    while (res == 0){
        SpinLock_Lock(&lock->writeLock);
        if (lock->writing==0){
            lock->readers++;
            res = 1;
        }
        SpinLock_Unlock(&lock->writeLock);
    }

}

inline void RwLock_ReadUnlock(struct RwLock* lock) {
    SpinLock_Lock(&lock->writeLock);
    lock->readers--;
    SpinLock_Unlock(&lock->writeLock);
}

inline void RwLock_WriteLock(struct RwLock* lock) {
    int res = 0;
    SpinLock_Lock(&lock->writeLock);
    lock->writing++;
    SpinLock_Unlock(&lock->writeLock);
    while (res==0){
        SpinLock_Lock(&lock->writeLock);
        if (lock->readers==0){
            res = 1;
        }
        SpinLock_Unlock(&lock->writeLock);
    }
}

inline void RwLock_WriteUnlock(struct RwLock* lock) {
    SpinLock_Lock(&lock->writeLock);
    lock->writing--;
    SpinLock_Unlock(&lock->writeLock);
}
