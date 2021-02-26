//
// Created by abel on 2/25/21.
//
#define _GNU_SOURCE

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdbool.h>
#include <limits.h>


static pthread_barrier_t start_barrier, end_barrier;

struct timespec time_measures[2][2];

#define NUMBER_OF_DEBUG_POINTS 3
struct timespec debug_time_measures[2][NUMBER_OF_DEBUG_POINTS];

void *thread_execution(void *data) {
    // Id of the process
    long process_id = (long) data;

    // Local debug time measures
    struct timespec local_debug_time_measures[NUMBER_OF_DEBUG_POINTS];

    // Time measures
    struct timespec start_time, end_time;

    // Synchronize both threads
    pthread_barrier_wait(&start_barrier);

    // Debug time get
    for (int i = 0; i < NUMBER_OF_DEBUG_POINTS; ++i) {
        clock_gettime(CLOCK_MONOTONIC, &(local_debug_time_measures[i]));
        sched_yield();
    }

    // Get actual time in this thread
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // sleep(1); // Force preemption
    sched_yield();

    // For debug purposes get the clock time
    clock_gettime(CLOCK_MONOTONIC, &end_time);

    // Wait until the end of the experiment
    pthread_barrier_wait(&end_barrier);
    time_measures[process_id][0] = start_time;
    time_measures[process_id][1] = end_time;


    for (int i = 0; i < NUMBER_OF_DEBUG_POINTS; ++i) {
        debug_time_measures[process_id][i] = local_debug_time_measures[i];
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

#define CORE_TO_TEST 3

#define MY_STACK_SIZE 0x4000

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
        if (pthread_attr_setstacksize(&(attr[i]), PTHREAD_STACK_MIN + MY_STACK_SIZE)) {
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
    if (munlockall())
        perror("munlockall failed");

    // Calculate differences
    struct timespec result;

    // Preemption time difference
    timespec_subtract(&result, &(time_measures[0][0]), &(time_measures[1][0]));

    // Print results
    long long preemption_cost_nanoseconds = result.tv_sec * 1000000000L + result.tv_nsec;

    printf("Time taken by the preemption %lld ns\n", preemption_cost_nanoseconds);
    printf("Trace:\n\tThread 1:\n\t\tTimestamp before preemption: %ld s and %ld ns\n\t\tTimestamp after preemption: %ld s and %ld ns\n\tThread 2:\n\t\tTimestamp before preemption: %ld s and %ld ns\n\t\tTimestamp after preemption: %ld s and %ld ns\n",
           time_measures[0][0].tv_sec, time_measures[0][0].tv_nsec,
           time_measures[0][1].tv_sec, time_measures[0][1].tv_nsec,
           time_measures[1][0].tv_sec, time_measures[1][0].tv_nsec,
           time_measures[1][1].tv_sec, time_measures[1][1].tv_nsec);

    printf("Debug points:\n\tThread 1:\n");
    for (int i = 0; i < NUMBER_OF_DEBUG_POINTS; ++i) {
        printf("\t\tPoint %d: %ld s and %ld ns\n", i, debug_time_measures[0][i].tv_sec,
               debug_time_measures[0][i].tv_nsec);
    }

    printf("Debug points:\n\tThread 2:\n");
    for (int i = 0; i < NUMBER_OF_DEBUG_POINTS; ++i) {
        printf("\t\tPoint %d: %ld s and %ld ns\n", i, debug_time_measures[1][i].tv_sec,
               debug_time_measures[1][i].tv_nsec);
    }

    if (timespec_subtract(&result, &(time_measures[0][1]), &(time_measures[1][0])) ||
        timespec_subtract(&result, &(time_measures[1][1]), &(time_measures[0][0])))
        printf("bad behaviour of the test\n");

}
