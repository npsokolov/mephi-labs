#pragma once
#include <mutex>
#include <condition_variable>
#include <optional>
class TimeOut : public std::exception
{
    const char *what() const noexcept override
    {
        return "Timeout";
    }
};
template <typename T>
class UnbufferedChannel
{
public:
    void Put(const T &data)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        has_value_.wait(lock, [&]
                        { return !(data_.has_value()); });
        data_.emplace(data);
        empty_.notify_one();
        NoOneGetting_.wait(lock);
        return;
    }
    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (empty_.wait_for(lock, timeout, [&]()
                            { return data_.has_value(); }))
        {
            auto x = data_.value();
            data_.reset();
            NoOneGetting_.notify_one();
            has_value_.notify_one();
            return x;
        }
        else
        {
            throw TimeOut();
        }
    }
private:
    std::condition_variable NoOneGetting_;
    std::condition_variable empty_;
    std::condition_variable has_value_;
    std::optional<T> data_;
    std::mutex mutex_;
};
