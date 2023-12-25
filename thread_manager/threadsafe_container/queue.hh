#pragma once

#include <queue>
#include <condition_variable> 
#include <mutex> 

namespace larva {
    template <typename T>
    class threadsafe_queue
    {
        std::queue<T>           _queue; 
        std::mutex              _mutex;
        std::condition_variable _cond; 

    public:
        T pop()
        {
            std::lock_guard<std::mutex> lock(this->_mutex);

            /* Unlock the lock guard and wait until the queue is not empty. */
            this->_cond.wait(lock, [this]() -> bool 
                                    {
                                        return !this->_queue.empty();
                                    });

            T item = this->_queue.front();
            this->_queue.pop();
            return item;         
        }

        void push(T item)
        {
            std::lock_guard<std::mutex> lock(this->_mutex);
            this->_queue.push(item);
            this->_cond.notify_one();
        }
    };
}