#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>

/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

class ThreadPool {
public:
    ThreadPool(size_t threadCount) : is_running(true) {
        for (size_t i = 0; i < threadCount; ++i) {
            threads_.emplace_back(&ThreadPool::Start, this);
        }
    }

    void PushTask(const std::function<void()>& task) {
        std::unique_lock<std::mutex> guard(mutex_);
        if (!is_running){
            throw std::runtime_error("Cannot push tasks after termination");
        }
        if (queue_.empty()) {
            queue_.push(move(task));
            condition_.notify_one();
        }else{
            queue_.push(move(task));
        }
    }

    void Terminate(bool wait) {
        is_running.store(false);
        if (!wait) {
            std::unique_lock<std::mutex> lock(mutex_);
            while (!queue_.empty()) {
                queue_.pop();
            }
        }
        condition_.notify_all();
        for (auto& thread : threads_) {
            thread.join();
        }

    }

    bool IsActive() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return is_running;
    }

    size_t QueueSize() const {
        std::lock_guard<std::mutex> guard(mutex_);
        return queue_.size();
    }

private:
    void Start()
    {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                condition_.wait(lock, [this]() { return !is_running || !queue_.empty(); });
                if (!is_running && queue_.empty()) {
                    return;
                }
                task = queue_.front();
                queue_.pop();
            }
            task();
        }
    }
    std::queue<std::function<void()>> queue_;
    mutable std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> is_running;
    std::vector<std::thread> threads_;
};