/***************************************************************************
*** MIT License ***
*
*** Copyright (c) 2020-2021 Daniel Mancuso - OhmTech.io **
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.     
****************************************************************************/

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

        //by now, we just drop new items if the queue is already full
        if (queue.size() < ++maxNumberOfItems) {
            queue.push(std::move(new_item));
            condition.notify_one();
        }    
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
