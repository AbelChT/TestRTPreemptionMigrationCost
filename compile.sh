#!/bin/bash
# None print version
aarch64-none-linux-gnu-g++ -static -O3 -D ANALIZE_PREEMPTION -o builds/main_preemption main.c
aarch64-none-linux-gnu-g++ -static -O3 -D ANALIZE_CONTINUOUS -o builds/main_continuous main.c
aarch64-none-linux-gnu-g++ -static -O3 -D ANALIZE_MIGRATION -o builds/main_migration main.c

# Print version
aarch64-none-linux-gnu-g++ -static -O3 -D PRINT_EXECUTION_TIME -D ANALIZE_PREEMPTION -o builds/main_preemption_print main.c
aarch64-none-linux-gnu-g++ -static -O3 -D PRINT_EXECUTION_TIME -D ANALIZE_CONTINUOUS -o builds/main_continuous_print main.c
aarch64-none-linux-gnu-g++ -static -O3 -D PRINT_EXECUTION_TIME -D ANALIZE_MIGRATION -o builds/main_migration_print main.c
