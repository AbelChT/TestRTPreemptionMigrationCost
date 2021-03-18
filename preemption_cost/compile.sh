#!/bin/bash

# Architecture variables
CC=aarch64-none-linux-gnu-gcc
ARCHITECTURE=aarch64

mkdir -p ../builds/${ARCHITECTURE}

# Get preemption cost
${CC} -Wall -static -pthread  -lpthread -o ../builds/${ARCHITECTURE}/preemption_cost preemption_cost_linux.c