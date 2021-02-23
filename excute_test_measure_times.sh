#!/bin/bash

rm continuous.csv preemption.csv migration.csv
NUMBER_OF_REPETITIONS=20

for ((i = 0; i < NUMBER_OF_REPETITIONS; i++)); do
  sleep 1
  echo "Continuous: Iteration $i / $NUMBER_OF_REPETITIONS"
  ./main_continuous_print >> continuous.txt
done

for ((i = 0; i < NUMBER_OF_REPETITIONS; i++)); do
  sleep 1
  echo "Preemption: Iteration $i / $NUMBER_OF_REPETITIONS"
  ./main_preemption_print >> preemption.txt
done

for ((i = 0; i < NUMBER_OF_REPETITIONS; i++)); do
  sleep 1
  echo "Migration: Iteration $i / $NUMBER_OF_REPETITIONS"
  ./main_migration_print >> migration.txt
done
