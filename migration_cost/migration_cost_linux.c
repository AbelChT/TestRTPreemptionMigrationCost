//
// Created by abel on 2/26/21.
//
#define _GNU_SOURCE

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>

// Define variables
#define NUMBER_OF_EXPERIMENTS 100
#define CORE_TO_TEST_INITIAL 2
#define CORE_TO_TEST_FINAL 3

bool timespec_subtract(struct timespec *result, struct timespec *start_time, struct timespec *end_time) {
    /***
     * Do timeval subtraction
     * result = end_time - start_time
     * Return true if is positive, else negative
     */

    bool return_value;
    if (end_time->tv_sec > start_time->tv_sec ||
        (end_time->tv_sec == start_time->tv_sec && end_time->tv_nsec >= start_time->tv_nsec)) {
        result->tv_sec = end_time->tv_sec - start_time->tv_sec;
        result->tv_nsec = end_time->tv_nsec - start_time->tv_nsec;
        return_value = true;
    } else {
        result->tv_sec = start_time->tv_sec - end_time->tv_sec;
        result->tv_nsec = start_time->tv_nsec - end_time->tv_nsec;
        return_value = false;
    }

    if (result->tv_nsec < 0) {
        result->tv_nsec += 1000000000L;
        result->tv_sec -= 1L;
    }

    return return_value;
}

int main() {
    // Set max priority for the thread
    // The sched fifo policy avoid involuntary preemption
    struct sched_param my_sched;
    my_sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
    if (sched_setscheduler(getpid(), SCHED_FIFO, &my_sched)) {
        perror("setscheduler failed");
        exit(-1);
    }

    // Now lock all current and future pages from preventing of being paged
    if (mlockall(MCL_CURRENT | MCL_FUTURE)) {
        perror("mlockall failed");
        exit(-1);
    }

    long long migration_cost_nanoseconds[NUMBER_OF_EXPERIMENTS];

    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        // Set sched initial affinity
        cpu_set_t mask_initial;
        CPU_ZERO(&mask_initial);
        CPU_SET(CORE_TO_TEST_INITIAL, &mask_initial);
        if (sched_setaffinity(0, sizeof(cpu_set_t), &mask_initial)) {
            perror("setaffinity failed");
            exit(-1);
        }

        // Set sched variables for final affinity
        cpu_set_t mask_final;
        CPU_ZERO(&mask_final);
        CPU_SET(CORE_TO_TEST_FINAL, &mask_final);

        // Variables where the time will be stored
        struct timespec local_time_measure_before, local_time_measure_after;

        // Get previous CPU for debug purposes
        int cpu_initial = sched_getcpu();

        // Get time of migration
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure_before));
        if (sched_setaffinity(0, sizeof(cpu_set_t), &mask_final)) {
            perror("setaffinity failed");
            exit(-1);
        }
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure_after));

        // Get posterior CPU for debug purposes
        int cpu_final = sched_getcpu();

        // Calculate differences
        struct timespec result;

        // Preemption time difference
        timespec_subtract(&result, &(local_time_measure_before), &(local_time_measure_after));

        // Local preemption cost
        migration_cost_nanoseconds[i] = result.tv_sec * 1000000000L + result.tv_nsec;

        // Check test behaviour
        if (cpu_initial != CORE_TO_TEST_INITIAL || cpu_final != CORE_TO_TEST_FINAL) {
            perror("bad behaviour of the test\n");
            exit(-1);
        }
    }

    // Unlock pages
    if (munlockall())
        perror("munlockall failed");

    // Analyze result
    long long min_migration_cost_nanoseconds, max_migration_cost_nanoseconds, avg_migration_cost_nanoseconds;

    // Max calculation
    max_migration_cost_nanoseconds = 0;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        if (max_migration_cost_nanoseconds < migration_cost_nanoseconds[i])
            max_migration_cost_nanoseconds = migration_cost_nanoseconds[i];
    }

    // Min calculation
    min_migration_cost_nanoseconds = max_migration_cost_nanoseconds;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        if (min_migration_cost_nanoseconds > migration_cost_nanoseconds[i])
            min_migration_cost_nanoseconds = migration_cost_nanoseconds[i];
    }

    // Avg calculation (Warning: If NUMBER_OF_EXPERIMENTS or the preemption cost is very high, there can be an integer
    // overflow in the variable avg_preemption_cost_nanoseconds)
    avg_migration_cost_nanoseconds = 0;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        avg_migration_cost_nanoseconds += migration_cost_nanoseconds[i];
    }
    avg_migration_cost_nanoseconds = avg_migration_cost_nanoseconds / NUMBER_OF_EXPERIMENTS;

    // Print result
    printf("Experiment result: \n\t%s: %d\n\t%s: %lld ns\n\t%s: %lld ns\n\t%s: %lld ns\n",
           "Number of experiments", NUMBER_OF_EXPERIMENTS,
           "Minimum cost of migration", min_migration_cost_nanoseconds,
           "Maximum cost of migration", max_migration_cost_nanoseconds,
           "Average cost of migration", avg_migration_cost_nanoseconds);

    return 0;
}
