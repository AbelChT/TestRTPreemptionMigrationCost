#!/bin/bash

#BINARIES_PATH="./builds/x86"

BINARIES_PATH="."

# Measure function
echo "Measuring preemption function"
trace-cmd record -p function ${BINARIES_PATH}/main_preemption
mv trace.dat preemption_function_trace.dat
sleep 1

echo "Measuring continuous function"
trace-cmd record -p function ${BINARIES_PATH}/main_continuous
mv trace.dat continuous_function_trace.dat
sleep 1

echo "Measuring migration function"
trace-cmd record -p function ${BINARIES_PATH}/main_migration
mv trace.dat migration_function_trace.dat
sleep 1

# Measure function graph
echo "Measuring preemption function graph"
trace-cmd record -p function_graph ${BINARIES_PATH}/main_preemption
mv trace.dat preemption_function_graph_trace.dat
sleep 1

echo "Measuring continuous function graph"
trace-cmd record -p function_graph ${BINARIES_PATH}/main_continuous
mv trace.dat continuous_function_graph_trace.dat
sleep 1

echo "Measuring migration function graph"
trace-cmd record -p function_graph ${BINARIES_PATH}/main_migration
mv trace.dat migration_function_graph_trace.dat
sleep 1

# Measure wake up
echo "Measuring preemption wakeup"
trace-cmd record -p wakeup ${BINARIES_PATH}/main_preemption
mv trace.dat preemption_wakeup_trace.dat
sleep 1

# Measure wake up
echo "Measuring preemption wakeup"
trace-cmd record -p wakeup ${BINARIES_PATH}/main_preemption
mv trace.dat preemption_wakeup_trace.dat
sleep 1

