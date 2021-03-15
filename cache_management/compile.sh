#!/bin/bash

CC=aarch64-none-linux-gnu-gcc
ARCHITECTURE=aarch64

#CC=g++
#ARCHITECTURE=x86

mkdir -p ../builds/${ARCHITECTURE}

# Get migration cost

${CC} -Wall -static -O0 -D AARCH64_COMPILATION l2_cache_fill.aarch64.S l2_cache_fill_cost.c -o ../builds/${ARCHITECTURE}/l2_cache_fill_cost

${CC} -Wall -static -O0 l2_cache_fill_cost.c -o ../builds/${ARCHITECTURE}/l2_cache_fill_cost_no_asm

${CC} -S -Wall -static -O0 l2_cache_fill_cost.c -o ../builds/${ARCHITECTURE}/l2_cache_fill_cost_no_asm.s

#${CC} -static -S -O0 -fverbose-asm -D AARCH64_COMPILATION -o ../builds/${ARCHITECTURE}/l2_cache_fill_cost.asm l2_cache_fill_cost.c