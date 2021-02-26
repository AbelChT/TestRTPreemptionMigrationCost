//
// Created by abel on 2/25/21.
//
// This program calculate the cost of the preemption in a Unix platform
//
#define _GNU_SOURCE

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>

// Define variables
#define NUMBER_OF_EXPERIMENTS 100
#define CORE_TO_TEST 3

// Barrier for the test
static pthread_barrier_t start_barrier, end_barrier;

// Data structures where the results will be stored
struct timespec time_measures[2][NUMBER_OF_EXPERIMENTS];
struct timespec debug_time_measures[2][NUMBER_OF_EXPERIMENTS];

void *thread_execution(void *data) {
    // Id of the process
    long process_id = (long) data;

    // Debug time get
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        // Define variables as locals to avoid cache fails while the measure
        struct timespec local_time_measure;
        struct timespec debug_local_time_measure;

        // Synchronize both threads
        pthread_barrier_wait(&start_barrier);

        // Get time (used to calculate preemption)
        clock_gettime(CLOCK_MONOTONIC, &(local_time_measure));
        sched_yield(); // Do context switch

        // Get time (used for debug purposes)
        clock_gettime(CLOCK_MONOTONIC, &(debug_local_time_measure));
        sched_yield(); // Do context switch

        // Synchronize both threads
        pthread_barrier_wait(&end_barrier);

        // Copy local to global data structures
        debug_time_measures[process_id][i] = debug_local_time_measure;
        time_measures[process_id][i] = local_time_measure;
    }

    return NULL;
}

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
    pthread_t threads[2];
    struct sched_param param[2];
    pthread_attr_t attr[2];
    cpu_set_t affinity_mask[2];

    // Initialize barriers
    if (pthread_barrier_init(&start_barrier, NULL, 2))
        perror("thread barrier initialization failed");

    if (pthread_barrier_init(&end_barrier, NULL, 2))
        perror("thread barrier initialization failed");

    // Lock memory
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
        perror("mlockall failed");
    }

    // Configure threads attributes
    for (int i = 0; i < 2; ++i) {
        // Init attrs
        if (pthread_attr_init(&(attr[i]))) {
            perror("pthread init failed");
            exit(-1);
        }

        // Set a specific stack size
        if (pthread_attr_setstacksize(&(attr[i]), PTHREAD_STACK_MIN + 0x4000)) {
            perror("pthread setstacksize failed");
            exit(-1);
        }

        // Set scheduler policy and priority of pthread
        if (pthread_attr_setschedpolicy(&(attr[i]), SCHED_FIFO)) {
            perror("pthread setschedpolicy failed");
            exit(-1);
        }
        param[i].sched_priority = sched_get_priority_max(SCHED_FIFO);

        if (pthread_attr_setschedparam(&(attr[i]), &(param[i]))) {
            perror("pthread setschedparam failed");
            exit(-1);
        }

        // Use scheduling parameters of attr
        if (pthread_attr_setinheritsched(&(attr[i]), PTHREAD_EXPLICIT_SCHED)) {
            perror("pthread setinheritsched failed");
            exit(-1);
        }

        // Set thread affinity
        CPU_ZERO(&(affinity_mask[i]));
        CPU_SET(CORE_TO_TEST, &(affinity_mask[i]));

        if (pthread_attr_setaffinity_np(&(attr[i]), sizeof(cpu_set_t), &(affinity_mask[i]))) {
            perror("pthread setaffinity failed");
            exit(-1);
        }
    }

    for (long i = 0; i < 2; i++)
        if (pthread_create(&threads[i], &(attr[i]), &thread_execution, (void *) i)) {
            perror("thread creation failed");
            exit(-1);
        }

    for (int i = 0; i < 2; i++)
        pthread_join(threads[i], NULL);

    // Unlock pages
    if (munlockall()) {
        perror("munlockall failed");
        exit(-1);
    }

    long long preemption_cost_nanoseconds[NUMBER_OF_EXPERIMENTS];

    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        // Calculate differences
        struct timespec result;

        // Preemption time difference
        timespec_subtract(&result, &(time_measures[0][i]), &(time_measures[1][i]));

        // Local preemption cost
        preemption_cost_nanoseconds[i] = result.tv_sec * 1000000000L + result.tv_nsec;

        // Check the correction of the test
        if (!timespec_subtract(&result, &(time_measures[0][i]), &(debug_time_measures[1][i])) ||
            !timespec_subtract(&result, &(time_measures[1][i]), &(debug_time_measures[0][i]))) {
            perror("bad behaviour of the test\n");

            printf("Test with error %d\n", i);

            // Print stack trace
            printf("Preemption points:\n\tThread 1:\n");
            for (int j = 0; j < NUMBER_OF_EXPERIMENTS; ++j) {
                printf("\t\tPoint %d: %ld s and %ld ns\n", j, time_measures[0][j].tv_sec,
                       time_measures[0][j].tv_nsec);
            }

            printf("Preemption points:\n\tThread 2:\n");
            for (int j = 0; j < NUMBER_OF_EXPERIMENTS; ++j) {
                printf("\t\tPoint %d: %ld s and %ld ns\n", j, time_measures[1][j].tv_sec,
                       time_measures[1][j].tv_nsec);
            }

            printf("Debug points:\n\tThread 1:\n");
            for (int j = 0; j < NUMBER_OF_EXPERIMENTS; ++j) {
                printf("\t\tPoint %d: %ld s and %ld ns\n", j, debug_time_measures[0][j].tv_sec,
                       debug_time_measures[0][j].tv_nsec);
            }

            printf("Debug points:\n\tThread 2:\n");
            for (int j = 0; j < NUMBER_OF_EXPERIMENTS; ++j) {
                printf("\t\tPoint %d: %ld s and %ld ns\n", j, debug_time_measures[1][j].tv_sec,
                       debug_time_measures[1][j].tv_nsec);
            }

            exit(-1);
        }
    }

    // Analyze result
    long long min_preemption_cost_nanoseconds, max_preemption_cost_nanoseconds, avg_preemption_cost_nanoseconds;

    // Max calculation
    max_preemption_cost_nanoseconds = 0;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        if (max_preemption_cost_nanoseconds < preemption_cost_nanoseconds[i])
            max_preemption_cost_nanoseconds = preemption_cost_nanoseconds[i];
    }

    // Min calculation
    min_preemption_cost_nanoseconds = max_preemption_cost_nanoseconds;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        if (min_preemption_cost_nanoseconds > preemption_cost_nanoseconds[i])
            min_preemption_cost_nanoseconds = preemption_cost_nanoseconds[i];
    }

    // Avg calculation (Warning: If NUMBER_OF_EXPERIMENTS or the preemption cost is very high, there can be an integer
    // overflow in the variable avg_preemption_cost_nanoseconds)
    avg_preemption_cost_nanoseconds = 0;
    for (int i = 0; i < NUMBER_OF_EXPERIMENTS; ++i) {
        avg_preemption_cost_nanoseconds += preemption_cost_nanoseconds[i];
    }
    avg_preemption_cost_nanoseconds = avg_preemption_cost_nanoseconds / NUMBER_OF_EXPERIMENTS;

    // Print result
    printf("Experiment result: \n\t%s: %d\n\t%s: %lld ns\n\t%s: %lld ns\n\t%s: %lld ns\n",
           "Number of experiments", NUMBER_OF_EXPERIMENTS,
           "Minimum cost of preemption", min_preemption_cost_nanoseconds,
           "Maximum cost of preemption", max_preemption_cost_nanoseconds,
           "Average cost of preemption", avg_preemption_cost_nanoseconds);

    return 0;
}
