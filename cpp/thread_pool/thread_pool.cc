#include <thread_pool.hh>
#include <stealing_thread_pool.hh>

thread_local std::unique_ptr<larva::thread_pool::local_queue_type> 
larva::thread_pool::_local_work_queue {nullptr};

thread_local larva::stealing_queue
*larva::stealing_thread_pool::_local_work_queue {nullptr};

thread_local unsigned larva::stealing_thread_pool::_index {0};