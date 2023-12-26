#include <iostream>
#include <thread_pool/thread_pool.hh>

int main() {
    larva::thread_pool pool;

    for (auto i : {1, 2, 3, 4, 5, 6}) {
        pool.submit([i]() -> void {
            std::cout << "Task " << i << " is running." << std::endl;
        });
    }

    while (1);
    
    return EXIT_SUCCESS;
}