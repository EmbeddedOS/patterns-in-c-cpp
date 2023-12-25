#pragma once
#include <atomic>

namespace larva {

class thread_pool {
    std::atomic_bool _done;
};

}