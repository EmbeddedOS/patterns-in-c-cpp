#pragma once
#include <atomic>
#include <threadsafe_container/queue.hh>

namespace larva {

class thread_pool {
    std::atomic_bool _done;
};

}