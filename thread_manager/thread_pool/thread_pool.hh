#pragma once
#include <atomic>
#include <functional>
#include <vector>
#include <thread>

#include <threadsafe_container/queue.hh>

namespace larva {

typedef std::function<void()> task_t;

class thread_pool {
    std::atomic_bool _done {false};
    larva::threadsafe_queue<larva::task_t> _work_queue {};
    std::vector<std::thread> _worker_threads {};

public:
    thread_pool()
    {
        unsigned const thread_number = std::thread::hardware_concurrency();
        try {
            for (unsigned i = 0; i < thread_number; ++i)
            {
                this->_worker_threads.push_back(
                            std::thread{&thread_pool::worker_thread, this});
            }
        } catch (...) {
            this->_done = true;
            throw;
        }
    }

    ~thread_pool() {
        this->_done = true;
    }


    template <typename FunctionType>
    void submit(FunctionType f)
    {
        this->_work_queue.push(larva::task_t(f));
    }

private:
    void worker_thread()
    {
        while (!this->_done) {
            larva::task_t task;
            if (this->_work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }   
};

}