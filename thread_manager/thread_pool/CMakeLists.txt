add_library(${THREAD_POOL_LIB} STATIC thread_pool.cc)
target_include_directories(${THREAD_POOL_LIB} PUBLIC "..")