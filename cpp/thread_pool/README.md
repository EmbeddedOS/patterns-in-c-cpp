# Advanced thread management

- We'll look at mechanisms for managing threads and tasks, starting with the automatic management of the number of threads and the division of tasks between them.

## 1. What is this?

- A thread-pool implementation based on cpp modern.

## 2. Thread pools

- In many companies, employees who would normally spend their time in the office are occasionally required to visit clients or suppliers or to attend a trade show or conference. Although these trips might be necessary, and on any given day there might be several people making this trip, it may well be months or even yeats between these trips for any particular employee.
- Because it would therefore be rather expensive and impractical for each employee to have a company car, companies often offer a **car pool** instead;
  - they have a limited number of cars that are available to all employees.
  - When an employee needs to make an off-site trip, they book one of the pool cars for the appropriate time and return it for others to use when they return to the office.
  - If there are no pool cars free on a given day, the employee will have to re-schedule their trip for a subsequent date.

- A **thread pool** i a similar idea, except that **threads** are being shared rather than cars.
  - On most systems, it's impractical to have a separate thread for every task that can potentially be done in parallel with other tasks, but you would still like to take advantage of the available concurrency where possible.
  - A Thread pool allows you to accomplish this; tasks that can be executed concurrently are submitted to the pool, which puts them on a **queue of pending work**. Each task is then taken from the queue by one of the **worker threads**, which executes the task before looping back to take another from the queue.

- There are several key design issues when building a thread pool, such as how many threads to use, the most efficient way to allocate tasks to threads, and wether or not you can wait for a task to complete.

### 2.2. The simplest possible thread pool

- At its simplest, a thread pool is a fixed number of **worker threads** (typically the same number as the value returned by `std::thread::hardware_concurrency()`) that process work.
- Each worker thread takes work off the queue, runs the specified task, and then goes back to the queue for more work. In the simplest case there's no way to wait for the task to complete.

```cpp
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
```

- The implementation has a vector of worker threads and uses one of the thread safe queues to manage the queue of work.
- Starting a thread can fail by throwing an exception, so you need to ensure that any threads you've already started are stopped and cleaned up nicely in this case. This is achieved with a *try-catch* block.

### 2.2. Waiting for tasks submitted to a thread pool

- With thread pools, you'd need to wait for the tasks submitted to the thread pool to complete, rather than the worker threads themselves. This is similar to the way that the `std::async` waited for the futures.
- This adds complexity to the code; it would be better if you could wait for the tasks directly. By moving that complexity into the thread pool itself, you can wait for the tasks directly. You can have the `submit()` function return a task handle of some description that you can then use to wait for the task complete. This task handle would wrap the use of condition variables or futures, simplifying the code that uses the thread pool.

- A special case of having to wait for the spawned task to finish occurs when the main thread needs a result computed by the task.

- Because the `std::packaged_task<>` instances are not **copyable**, just **movable**, we can no longer use `std::function<>` for the queue entries, because `std::function<>` requires that the stored function objects are copy-constructible. Instead, we must use a custom function wrapper that can handle move-only types. This is a simple type-erasure class with a function call operator. We only need to handle functions that take no parameters and return void, so this is a straightforward virtual call in the implementation.

```cpp
#pragma once

#include <future>
#include <memory>

namespace larva {

    class f_wrapper {
        struct impl_base {
            virtual void call() = 0;
            virtual ~impl_base() {}
        };

        template <typename F>
        struct impl: impl_base {
            F _f;
            impl(F&& f): _f {std::move(f)} {}
            void call() { this->_f(); }
        };

        std::unique_ptr<impl_base> _impl {nullptr};

    public:
        template <typename F>
        f_wrapper(F&& f): _impl {std::make_unique<impl<F>>(std::move(f))} {}
        f_wrapper(f_wrapper&& other): _impl {std::move(other._impl)} {}
        f_wrapper() = default;

        void operator() ()
        {
            this->_impl->call();
        }

        f_wrapper& operator=(f_wrapper&& other)
        {
            this->_impl = std::move(other._impl);
        }

        f_wrapper(const f_wrapper&) = delete;
        f_wrapper(f_wrapper&) = delete;
        f_wrapper& operator=(const f_wrapper&) = delete;
    };

}
```

```cpp
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
    std::future<typename std::result_of<FunctionType()>::type>
    submit(FunctionType f)
    {
        typedef typename std::result_of<FunctionType()>::type result_type;
        std::packaged_task<result_type()> task(std::move(f));
        std::future<result_type> res(task.get_future());
        
        this->_work_queue.push(std::move(task));

        return res;
    }

private:
    void worker_thread()
    {
        while (!this->_done) {
            larva::f_wrapper task;
            if (this->_work_queue.try_pop(task)) {
                task();
            } else {
                std::this_thread::yield();
            }
        }
    }   
};

}
```

- First, the modified `submit()` function returns a `std::future<>` to hold the return value of the task and allow the caller to wait for the task complete. This requires that you know the return type of the supplied function `f`, which is where `std::result_of<>` comes in: `std::result_of<FunctionType()>::type` is the type of the result of invoking an instance of type `FunctionType` (such as `f`) with no arguments. You use the same `std::result<>` expression for the `result_type typedef` inside function.

- We then wrap the function `f` in a `std::packaged-task<result_type()>`, because `f` is a function or callable object that takes no parameters and return an instance of type `result_type`, as we deduced. We can now get your future from `std::packaged-task<>` before pushing the task onto the queue and returning the future.

- Note that you have to use `std::move()` when pushing the task onto the queue, because the `std::packaged-task<>` isn't **copyable**. The queue now store `f_wrapper` objects rather than `std::function<void()>` objects in order to handle this.

- This works well for simple cases, where the tasks are independent. But it's not so good for situations where the tasks depend on other tasks also submitted to the thread pool.

### 2.4. Avoiding contention on the work queue

- Every time a thread calls `submit()` on a particular instance of the thread pool, it has to push a new item onto the single shared work queue. Likewise, the worker threads are continually popping items off the queue in order to run the tasks. This means that as the number of processors increases, there's increasing contention on the queue. **This can be a real performance drain;** even if you use a lock-free queue so there's no explicit waiting, *cache ping-pong* can be a substantial time sink.

- One way to avoid *cache ping-pong* is to **use a separate work queue per thread**. Each thread then posts new items to its own queue and takes work from the global work queue only if there's no work on its own individual queue.

- We use `thread_local` variable to ensure that each thread has its own work queue, as well as the global.

```cpp
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
```

- We've used a `std::unique_ptr<>` to hold the thread-local work queue because we don't want other threads that aren't part of your thread pool to have one; this is initialized in the `worker_thread()` function before the processing loop. The destructor of `std::unique_ptr<>` will ensure that the work queue is destroyed when the thread exits.

- `submit()` then checks to see if the current thread has a work queue. If it does, it's a pool thread, and you can put the task on the local queue; otherwise you need to put the task on the pool queues.

- This works fine for reducing contention, but when the distribution of work is uneven, it can easily result in one thread having a lot of work in its queue while the others have no work to do. **This defeats the purpose of using a thread pool**.

- Thankfully, there is a solution to this: allow the threads to `steal` work from each other's queues if there's no work in their queue and no work in the global queue.

### 2.5. Work stealing

- In order to allow a thread with no work to do to take work from another thread with a full queue, the queue must be accessible to the thread doing the stealing from `run_pending_tasks()`. This requires that each thread register its queue with the thread pool or be given one by the thread pool. Also, you must ensure that the data in the work queue is suitably synchronized and protected so that your invariants are protected.

- It's possible to write a lock-free queue that allows the owner thread to push and pop at one end while other threads can steal entries from the other.

```cpp
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
```

- This queue is a simple wrapper around a `std::deque<f_wrapper>` that protects all accesses with a mutex lock. Both `push()` and `try_pop()` work on the front of the queue, while `try_steal()` work on the back.

- This means that this *queue* is a *last-in-first-out* stack for its own thread; the task most recently pushed on is the first one off again. This can help improve performance from a cache perspective, because the data related to a task pushed on the queue previously.

- `try_steal()` takes items from the opposite end of the queue to `try_pop()` in order to minimize contention;

```cpp
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
```

- This code is similar with `thread_pool`. The first difference is that each thread has a `stealing_queue` rather than a plain `std::queue<>`. When each thread is created, rather than allocating its own work queue, the pool constructor allocates one, which is then stored in the list of work queues for this pool. The index of the queue in the list is then passed in to the thread function and used to retrieve the pointer to the queue. This means that the thread pool can access the queue when trying to steal a task for a thread that has no work to do.

- `run_pending_task()` will now try to take task from its thread's own queue, take a task from the pool queue, or take a task from the queue of another thread.

- `pop_task_from_other_thread_queue()` iterates through the queues belonging to all the threads in the pool, trying to steak a task from each in turn. In order to avoid every thread trying to steal from the first thread in the lest, each thread starts at the next thread in the list by offsetting the index of the queue to check by its own index.

- Now we have a working thread pool that's good for many potential uses. One aspect that hasn't been explored is the idea of dynamically resizing the thread pool to ensure that there's optimal CPU usage even when threads are blocked waiting for something such as I/O or a mutex lock.
