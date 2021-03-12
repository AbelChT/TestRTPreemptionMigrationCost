cmake_minimum_required(VERSION 3.17)
project(StressCPUC C)

set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(StressCPUC cache_management/l2_cache_fill_cost.c)
target_link_libraries(StressCPUC PRIVATE Threads::Threads)