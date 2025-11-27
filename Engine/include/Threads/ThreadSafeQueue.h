#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
public:
    ThreadSafeQueue() = default;

    void push(T item)
    {
        std::unique_lock<std::mutex> lock(mutex);
        if (queue.size() >= MAX_HISTORY) {
            queue.pop();
        }
        queue.push(std::move(item));
        cond.notify_one();
    }

    T pop()
    {
        std::unique_lock<std::mutex> lock(mutex);

        cond.wait(lock,
            [this]() { return !queue.empty(); });

        T item = std::move(queue.front());
        queue.pop();

        return item;
    }

    bool try_pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex);

        if (queue.empty())
            return false;

        item = std::move(queue.front());
        queue.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex);
        return queue.size();
    }
private:
    const uint8_t MAX_HISTORY = 64;
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable cond;
};