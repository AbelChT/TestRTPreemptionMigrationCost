#!/bin/bash

CC=aarch64-none-linux-gnu-g++
ARCHITECTURE=aarch64

#CC=g++
#ARCHITECTURE=x86

mkdir -p ../builds/${ARCHITECTURE}

# Get migration cost
${CC} -static -O0 -o ../builds/${ARCHITECTURE}/l2_cache_fill_cost l2_cache_fill_cost.c
