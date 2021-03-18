#!/bin/bash

CC=aarch64-none-linux-gnu-gcc
ARCHITECTURE=aarch64

mkdir -p ../builds/${ARCHITECTURE}

# Get MP to L2 transfer cost
#
${CC} -Wall -O0 -D AARCH64_COMPILATION l2_cache_fill.aarch64.S l2_cache_fill_cost.c -o ../builds/${ARCHITECTURE}/l2_cache_fill_cost