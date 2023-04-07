#pragma once
#include <atomic>

class SharedMutex {
public:

    void lock() {
        while (UniqueLocked_.exchange(true)){}
        while (SharedLocked_.load()) {}
    }

    void unlock() {
        UniqueLocked_.store(false);
    }

    void lock_shared() {
        while (UniqueLocked_.load()) {}
        SharedLocked_.store(true);
        ReaderCount_.fetch_add(1);
    }

    void unlock_shared() {
        if (ReaderCount_.fetch_sub(1)==1){
            SharedLocked_.store(false);
        }
    }

private:
    std::atomic<int> ReaderCount_{0};
    std::atomic<bool> SharedLocked_{false};
    std::atomic<bool> UniqueLocked_{false};
};