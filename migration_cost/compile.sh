#!/bin/bash

CC=aarch64-none-linux-gnu-g++
ARCHITECTURE=aarch64

#CC=g++
#ARCHITECTURE=x86

mkdir -p builds/${ARCHITECTURE}

# Get migration cost
${CC} -static -pthread -lpthread -o builds/${ARCHITECTURE}/migration_cost migration_cost_linux.c
