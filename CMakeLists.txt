cmake_minimum_required(VERSION 3.17)
project(StressCPUC C)

set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(StressCPUC get_preemption_time_linux_all_lock.c)
target_link_libraries(StressCPUC PRIVATE Threads::Threads)