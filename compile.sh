#!/bin/bash

CC=aarch64-none-linux-gnu-g++
ARCHITECTURE=aarch64

#CC=g++
#ARCHITECTURE=x86

mkdir -p builds/${ARCHITECTURE}

# None print version
${CC} -static -O3 -D ANALIZE_PREEMPTION -o builds/${ARCHITECTURE}/main_preemption main.c
${CC} -static -O3 -D ANALIZE_CONTINUOUS -o builds/${ARCHITECTURE}/main_continuous main.c
${CC} -static -O3 -D ANALIZE_MIGRATION -o builds/${ARCHITECTURE}/main_migration main.c

# Print version
${CC} -static -O3 -D PRINT_EXECUTION_TIME -D ANALIZE_PREEMPTION -o builds/${ARCHITECTURE}/main_preemption_print main.c
${CC} -static -O3 -D PRINT_EXECUTION_TIME -D ANALIZE_CONTINUOUS -o builds/${ARCHITECTURE}/main_continuous_print main.c
${CC} -static -O3 -D PRINT_EXECUTION_TIME -D ANALIZE_MIGRATION -o builds/${ARCHITECTURE}/main_migration_print main.c
