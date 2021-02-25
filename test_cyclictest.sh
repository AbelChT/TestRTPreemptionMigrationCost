#!/bin/bash

# Latencies are in microseconds
cyclictest -a 2,3 -p 95 -D 1m -mlockall -q >preemption_test.txt

# Latencies are in microseconds
rt-migrate-test >migration_test.txt
