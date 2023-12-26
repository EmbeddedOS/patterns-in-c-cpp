#pragma once
#include <atomic>
#include <functional>
#include <vector>
#include <thread>

#include <threadsafe_container/queue.hh>
#include <f_wrapper.hh>

namespace larva {

typedef std::function<void()> task_t;

class thread_pool {
    std::atomic_bool _done {false};
    larva::threadsafe_queue<larva::f_wrapper> _work_queue {};
    std::vector<std::thread> _worker_threads {};

    typedef std::queue<larva::f_wrapper> local_queue_type;

    static thread_local
    std::unique_ptr<local_queue_type> _local_work_queue;

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

    ~thread_pool()
    {
        this->_done = true;
    }


    template <typename FunctionType>
    std::future<typename  std::result_of<FunctionType()>::type>
    submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        
        /* If Local pending task is initialized, we push task on it, otherwise,
         * we push on the shared queue. */
        if (this->_local_work_queue) {
            this->_local_work_queue->push(std::move(task));
        } else {
            this->_work_queue.push(std::move(task));
        }

        return res;
    }

    void run_pending_task()
    {
        larva::f_wrapper task;
        if (this->_local_work_queue && !this->_local_work_queue->empty()) {
            task = std::move(this->_local_work_queue->front());
            this->_local_work_queue->pop();
            task();
        } else if (this->_work_queue.try_pop(task)) {
            task();
        } else {
            std::this_thread::yield();
        }  
    }

private:
    void worker_thread()
    {
        this->_local_work_queue.reset(new local_queue_type);

        while (!this->_done) {
            this->run_pending_task();
        }
    }   
};

}