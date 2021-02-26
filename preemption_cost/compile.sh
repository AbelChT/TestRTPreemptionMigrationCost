#!/bin/bash

# Architecture variables
#CC=aarch64-none-linux-gnu-g++
#ARCHITECTURE=aarch64

CC=g++
ARCHITECTURE=x86

mkdir -p ../builds/${ARCHITECTURE}

# Get preemption cost
${CC} -static -pthread  -lpthread -o ../builds/${ARCHITECTURE}/preemption_cost preemption_cost_linux.c