#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template<typename T>
class TQueue
{
public:    
    TQueue() = default;
    TQueue(const TQueue&) = delete;            // disable copying
    TQueue& operator=(const TQueue&) = delete; // disable assignment
    ~TQueue() = default;

    void setMaxSize(size_t max_size) {maxNumberOfItems = max_size;}

    void push(T&& new_item)
    {
        std::lock_guard<std::mutex> mlock(mutex);

        queue.push(std::move(new_item));
        condition.notify_one();
    }

    /* Blocking */
    // T pop()
    // {
    //     std::unique_lock<std::mutex> mlock(mutex);

    //     while(queue.empty())
    //     {
    //         condition.wait(mlock);
    //     }
    //     T val = std::move(queue.front());
    //     queue.pop();
    //     return std::move(val);
    // }
    T pop()
    {
        std::unique_lock<std::mutex> mlock(mutex);

        while(queue.empty())
        {
            condition.wait(mlock);
        }
        T val = std::move(queue.front());
        queue.pop();
        return val;
    }

    /* Non-blocking */
    bool tryPop (T& item)
    {
        std::unique_lock<std::mutex> mlock(mutex);

        if (queue.empty())
            return false;

        item = queue.front();
        queue.pop();
        return true;
    }

private:
    size_t maxNumberOfItems = 10;
    std::queue<T> queue;
    mutable std::mutex mutex;
    std::condition_variable condition;
};
