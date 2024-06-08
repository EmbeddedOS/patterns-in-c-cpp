#pragma once
#include <f_wrapper.hh>
#include <queue>
#include <mutex>

namespace larva {
    typedef f_wrapper data_type;

    class stealing_queue {
        std::deque<data_type> _queue;
        mutable std::mutex _mutex; /* Change mutex in const method. */
    
    public:
        stealing_queue() = default;
        stealing_queue(const stealing_queue& other) = delete;
        stealing_queue& operator=(const stealing_queue& other) = delete;

        void push(data_type data) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            this->_queue.push_front(std::move(data));
        }

        bool empty() const {
            std::lock_guard<std::mutex> lock(this->_mutex);
            return this->_queue.empty();
        }

        bool try_pop(data_type& res) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            if (this->_queue.empty()) {
                return false;
            }

            res = std::move(this->_queue.front());
            this->_queue.pop_front();
            return true;
        }

        bool try_steal(data_type& res) {
            std::lock_guard<std::mutex> lock(this->_mutex);
            if (this->_queue.empty()) {
                return false;
            }

            res = std::move(this->_queue.back());
            this->_queue.pop_back();
            return true;
        }
    };
}