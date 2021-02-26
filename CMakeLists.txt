cmake_minimum_required(VERSION 3.17)
project(StressCPUC C)

set(CMAKE_C_STANDARD 11)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(StressCPUC preemption_cost/preemption_cost_linux.c migration_cost/migration_cost_linux.c)
target_link_libraries(StressCPUC PRIVATE Threads::Threads)