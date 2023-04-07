#include "task.h"
#include <vector>
#include <iostream>
#include <thread>

PrimeNumbersSet::PrimeNumbersSet()
{
    nanoseconds_under_mutex_ = 0;
    nanoseconds_waiting_mutex_ = 0;
}

bool PrimeNumbersSet::IsPrime(uint64_t number) const
{
    set_mutex_.lock();
    bool flag = (primes_.find(number)!=primes_.end());
    set_mutex_.unlock();
    return flag;
}

uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const
{
    set_mutex_.lock();
    auto s = primes_.upper_bound(number);
    if (s!=primes_.end()){
        set_mutex_.unlock();
        return *s;
    }else{
        set_mutex_.unlock();
        throw std::invalid_argument("");
    }
}

void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to)
{
    std::vector<bool> prime(to,true);
    prime[0]=false;
    prime[1]=false;
    std::atomic<int> i,j;
    for (i = 2; i*i < int(to); ++i) {
        if (prime[i]){
            for (j = i*i; j*j < int(to); j+=i) {
                prime[j]=false;
            }
        }
    }
    for (i = 2; i*i < int(to); ++i)
    {
        if (prime[i]) {
            for (j = std::max(i*i,(int(from)+i-1)/i*i); j < int(to); j+=i) {
                prime[j]=false;
            }

        }
    }
    std::chrono::time_point<std::chrono::steady_clock> final,start,start1;
    for (i = int(from); i<int(to); i++)
    {

        if (prime[i]){
            start = std::chrono::steady_clock::now();
            set_mutex_.lock();
            start1 = std::chrono::steady_clock::now();
            primes_.insert(i);
            final = std::chrono::steady_clock::now();
            set_mutex_.unlock();
            nanoseconds_waiting_mutex_+=std::chrono::duration_cast<std::chrono::nanoseconds>(start1-start).count();
            nanoseconds_under_mutex_+=std::chrono::duration_cast<std::chrono::nanoseconds>(final-start1).count();
        }
    }
}

size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const
{
    set_mutex_.lock();
    size_t size = primes_.size();
    set_mutex_.unlock();
    return size;

}

uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const
{
    set_mutex_.lock();
    if (primes_.empty()){
        set_mutex_.unlock();
        return 0;
    }
    uint64_t mx = *(--primes_.end());
    set_mutex_.unlock();
    return mx;
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const
{
    return std::chrono::nanoseconds(nanoseconds_waiting_mutex_);
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const
{
    return std::chrono::nanoseconds(nanoseconds_under_mutex_);
}
