//
// Created by abel on 3/12/21.
//

#define _GNU_SOURCE

#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <sched.h>
#include <unistd.h>

#define NUMBER_OF_EXPERIMENTS 100
#define CORE_TO_TEST 2

#define L1_CACHE_SIZE_BYTES 16384 // 16KB L1P and L1D
#define L2_CACHE_SIZE_BYTES 524288 // 512KB L2

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
    // Variables where the time will be stored
    struct timespec local_time_measure_before, local_time_measure_after;

    // Variable with the size of half L2 cache
    // As other programs can be executing concurrently, only half cache will be used for the tests.
    // This will correspond to 8 from the 16 lines each set has
    int8_t l2_fill_vector[L2_CACHE_SIZE_BYTES / 2];

    // Auxiliary variable where the time difference will be stored. It shouldn't be stored in cache while the loop execution
    struct timespec result;

    // Variables that will contain the execution time. They shouldn't be stored in cache while the loop execution neither
    long long cached_vector_operation_time, not_cached_vector_operation_time;

    // Vector were times will be stored
    long long l2_load_cost_nanoseconds[NUMBER_OF_EXPERIMENTS];

    // Register used to load memory
    register int8_t load_register;

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

    // Set sched affinity to some CPU
    cpu_set_t mask_cpu;
    CPU_ZERO(&mask_cpu);
    CPU_SET(CORE_TO_TEST, &mask_cpu);
    if (sched_setaffinity(0, sizeof(cpu_set_t), &mask_cpu)) {
        perror("setaffinity failed");
        exit(-1);
    }

    // Execute experiments
    for (int j = 0; j < NUMBER_OF_EXPERIMENTS; ++j) {
        // Fill L2 level cache
        for (int i = 0; i < L2_CACHE_SIZE_BYTES / 2; i++) {
            load_register = l2_fill_vector[i];
        }

        // Cost of load vector stored in L2Cache
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure_after));
        for (int i = 0; i < L2_CACHE_SIZE_BYTES / 2; i++) {
            load_register = l2_fill_vector[i];
        }
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure_before));

        // Operation time calculation
        timespec_subtract(&result, &(local_time_measure_before), &(local_time_measure_after));
        cached_vector_operation_time = result.tv_sec * 1000000000L + result.tv_nsec;

        // Clean cache
        __builtin___clear_cache(l2_fill_vector, l2_fill_vector + sizeof(l2_fill_vector));

        // Cost of load vector not stored in L2Cache
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure_after));
        for (int i = 0; i < L2_CACHE_SIZE_BYTES / 2; i++) {
            load_register = l2_fill_vector[i];
        }
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure_before));

        // Operation time calculation
        timespec_subtract(&result, &(local_time_measure_before), &(local_time_measure_after));
        not_cached_vector_operation_time = result.tv_sec * 1000000000L + result.tv_nsec;

        // Store experiment result
        l2_load_cost_nanoseconds[j] = not_cached_vector_operation_time - cached_vector_operation_time;
    }

    // Unlock pages
    if (munlockall())
        perror("munlockall failed");

    // Analyze result
    long long min_l2_fill_cost_nanoseconds, max_l2_fill_cost_nanoseconds, avg_l2_fill_cost_nanoseconds;

    // Max calculation
    max_l2_fill_cost_nanoseconds = 0;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        if (max_l2_fill_cost_nanoseconds < l2_load_cost_nanoseconds[i])
            max_l2_fill_cost_nanoseconds = l2_load_cost_nanoseconds[i];
    }

    // Min calculation
    min_l2_fill_cost_nanoseconds = max_l2_fill_cost_nanoseconds;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        if (min_l2_fill_cost_nanoseconds > l2_load_cost_nanoseconds[i])
            min_l2_fill_cost_nanoseconds = l2_load_cost_nanoseconds[i];
    }

    // Avg calculation (Warning: If NUMBER_OF_EXPERIMENTS or the fill cost is very high, there can be an integer
    // overflow in the variable avg_preemption_cost_nanoseconds)
    avg_l2_fill_cost_nanoseconds = 0;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        avg_l2_fill_cost_nanoseconds += l2_load_cost_nanoseconds[i];
    }
    avg_l2_fill_cost_nanoseconds = avg_l2_fill_cost_nanoseconds / NUMBER_OF_EXPERIMENTS;

    // Print result
    printf("Experiment result: \n\t%s: %d\n\t%s: %lld ns\n\t%s: %lld ns\n\t%s: %lld ns\n",
           "Number of experiments", NUMBER_OF_EXPERIMENTS,
           "Minimum cost of fill half l2 cache", min_l2_fill_cost_nanoseconds,
           "Maximum cost of fill half l2 cache", max_l2_fill_cost_nanoseconds,
           "Average cost of fill half l2 cache", avg_l2_fill_cost_nanoseconds);
}
