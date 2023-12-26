#include <thread_pool/thread_pool.hh>

thread_local std::unique_ptr<larva::thread_pool::local_queue_type> 
larva::thread_pool::_local_work_queue {nullptr};