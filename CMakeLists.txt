# 0.) Create source and Cmake file.
# 1.) mkdir build
# 2.) cd build
# 3.) cmake ..  - Generating the Build Files / Configure the project.
# 4.) cmake --build .


cmake_minimum_required(VERSION 3.10)

project(ThreadPool
        LANGUAGES C CXX
        VERSION 1.0.0)

set(CMAKE_CXX_STANDARD                  17)
set(CMAKE_CXX_STANDARD_REQUIRED         ON)

set(THREAD_POOL_LIB thread_pool)

option(COMPILE_TEST "Whether to compile the test" OFF)

add_subdirectory(thread_manager/thread_pool)

if (COMPILE_TEST)
        add_subdirectory(test)
else()
        message("W/o exe. Compiling...")
endif()