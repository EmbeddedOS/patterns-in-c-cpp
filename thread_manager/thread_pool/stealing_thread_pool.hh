#pragma once
#include <atomic>
#include <functional>
#include <vector>
#include <thread>

#include <threadsafe_container/stealing_queue.hh>
#include <threadsafe_container/queue.hh>
#include <thread_pool/joiner_thread.hh>
#include <f_wrapper.hh>

namespace larva {

    class stealing_thread_pool {
        std::atomic_bool _done {false};
        larva::threadsafe_queue<larva::f_wrapper> _work_queue {};
        std::vector<std::thread> _worker_threads {};
        larva::join_threads _joiner;
        std::vector<std::unique_ptr<larva::stealing_queue>> _queues {};
        static thread_local larva::stealing_queue *_local_work_queue;
        static thread_local unsigned _index;

    public:
        stealing_thread_pool(): _joiner {this->_worker_threads}, _done {false}
        {
            unsigned const thread_number = std::thread::hardware_concurrency();
            try {
                for (unsigned i = 0; i < thread_number; ++i)
                {
                    this->_queues.push_back(
                        std::make_unique<larva::stealing_queue>());

                    this->_worker_threads.push_back(
                        std::thread{&stealing_thread_pool::worker_thread,
                                    this, i});
                }
            } catch (...) {
                this->_done = true;
                throw;
            }
        }

        ~stealing_thread_pool()
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
            
            /* If Local pending task is initialized, we push task on it,
            *  otherwise, we push on the shared queue. */
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
            if (this->pop_task_from_local_queue(task)
                || this->pop_task_from_pool_queue(task)
                || this->pop_task_from_other_thread_queue(task))
            {
                    task();
            } else {
                std::this_thread::yield();
            }

        }

    private:
        void worker_thread(unsigned index)
        {
            this->_index = index;
            this->_local_work_queue = this->_queues[this->_index].get();

            while (!this->_done) {
                this->run_pending_task();
            }
        }

        bool pop_task_from_pool_queue(f_wrapper &task)
        {
            return this->_work_queue.try_pop(task);
        }

        bool pop_task_from_local_queue(f_wrapper &task)
        {
            return this->_local_work_queue
                    && this->_local_work_queue->try_pop(task);
        }

        bool pop_task_from_other_thread_queue(f_wrapper &task)
        {
            for (unsigned i = 0; i < this->_queues.size(); i++) {
                /* Current thread will try to steal task from next thread.
                 * We do that to avoid every threads steal from first thread. */
                unsigned index_of_other = (this->_index + i + 1)
                                            % this->_queues.size();
                if (this->_queues[index_of_other]->try_steal(task))
                {
                    return true;
                }
            }

            return false;
        }
    };

}