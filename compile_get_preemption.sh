#!/bin/bash

#CC=aarch64-none-linux-gnu-g++
#ARCHITECTURE=aarch64

CC=g++
ARCHITECTURE=x86

mkdir -p builds/${ARCHITECTURE}

# Get preemption aarch64
${CC} -static -pthread  -lpthread -o builds/${ARCHITECTURE}/get_preemption_time_linux_o0 get_preemption_time_linux.c