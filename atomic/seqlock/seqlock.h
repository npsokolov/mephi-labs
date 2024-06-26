#pragma once

#include <stdint.h>
#include <stdatomic.h>

inline void AtomicAdd(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock add %1, %0\n\t"
    :"+m" (*atomic)
    :"r"(value)
    :"memory"
    );
    __sync_synchronize();
}
inline void AtomicSub(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock sub %1, %0\n\t"
    :"+m" (*atomic)
    :"r"(value)
    :"memory"
    );
    __sync_synchronize();
}

inline int64_t AtomicXchg(int64_t* atomic, int64_t value) {
    asm volatile (
        "lock xchg %1, %0\n\t"
    : "+m" (*atomic), "+r" (value)
    :
    :"memory"
    );
    __sync_synchronize();
    return value;
}


struct SpinLock {
    int64_t locked;
};

inline void SpinLock_Init(struct SpinLock* lock) {
    lock->locked = 0;
    __sync_synchronize();
}

inline void SpinLock_Lock(struct SpinLock* lock) {
    while (AtomicXchg(&lock->locked,1)) {}
}

inline void SpinLock_Unlock(struct SpinLock* lock) {
    AtomicSub(&lock->locked,1);
}


struct SeqLock {
    int64_t counter;
    struct SpinLock spinLock;
};

inline void SeqLock_Init(struct SeqLock* lock) {
    lock->counter = 0;
    SpinLock_Init(&lock->spinLock);
    __sync_synchronize();
}

int64_t SeqLock_ReadLock(struct SeqLock *lock) {
    int64_t val;
    do{
        val = lock->counter;
        __sync_synchronize();
    }while(val!=lock->counter);
    return val;
}

int SeqLock_ReadUnlock(struct SeqLock *lock, int64_t prev) {
    __sync_synchronize();
    return (prev%2==0 && lock->counter==prev);
}

void SeqLock_WriteLock(struct SeqLock *lock) {
    SpinLock_Lock(&lock->spinLock);
    AtomicAdd(&lock->counter,1);
}

void SeqLock_WriteUnlock(struct SeqLock *lock) {
    AtomicAdd(&lock->counter,1);
    SpinLock_Unlock(&lock->spinLock);
}
