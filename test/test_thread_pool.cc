#include <iostream>
#include <thread_pool/thread_pool.hh>
#include <thread_pool/stealing_thread_pool.hh>

int main() {
    larva::stealing_thread_pool pool;

    for (auto i : {1, 2, 3, 4, 5, 6}) {
        std::future<int> ret = pool.submit([i]() -> int {
            std::cout << "Task " << i << " is running." << std::endl;
            return i + 1;
        });

        std::cout << "Get task result: " << ret.get() << std::endl;
    }

    while (1);
    
    return EXIT_SUCCESS;
}