#pragma once
#include <vector>
#include <thread>

namespace larva {

    /**
     * @brief       - This class automatically join all threads when the joiner
     *                object is destroyed.
     */
    class join_threads {
        std::vector<std::thread>& _threads;
    
    public:
        explicit join_threads(std::vector<std::thread>& threads):
            _threads {threads} {}

        ~join_threads()
        {
            for (auto &thread: this->_threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }
    };
}