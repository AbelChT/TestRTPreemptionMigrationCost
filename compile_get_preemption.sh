#!/bin/bash

CC=aarch64-none-linux-gnu-g++
ARCHITECTURE=aarch64

#CC=g++
#ARCHITECTURE=x86

mkdir -p builds/${ARCHITECTURE}

# Print version
${CC} -static -pthread  -lpthread -O0 -o builds/${ARCHITECTURE}/get_preemption_time_linux_o0 get_preemption_time_linux.c
${CC} -static -pthread  -lpthread -O1 -o builds/${ARCHITECTURE}/get_preemption_time_linux_o1 get_preemption_time_linux.c
${CC} -static -pthread  -lpthread -O2 -o builds/${ARCHITECTURE}/get_preemption_time_linux_o2 get_preemption_time_linux.c
${CC} -static -pthread  -lpthread -O3 -o builds/${ARCHITECTURE}/get_preemption_time_linux_o3 get_preemption_time_linux.c