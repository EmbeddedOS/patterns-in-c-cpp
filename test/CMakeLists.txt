add_executable(test_thread_pool.exe test_thread_pool.cc)
target_link_libraries(test_thread_pool.exe PUBLIC ${THREAD_POOL_LIB})
target_include_directories(test_thread_pool.exe PUBLIC "../thread_manager")