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

#define NUMBER_OF_TESTS 12

static pthread_barrier_t start_barrier, end_barrier;

/**
 * Set total affinity to CPU
 * @param cpu_where_set cpu where affinity will be set
 * @param previous_mask mask before affinity
 */
int set_affinity_to_cpu(unsigned long cpu_where_set) {
    cpu_set_t previous_mask;
    long n_processors = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_where_set >= n_processors || sched_getaffinity(0, sizeof(cpu_set_t), &previous_mask) == -1) {
        return -1;
    } else {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(cpu_where_set, &mask);
        return sched_setaffinity(0, sizeof(cpu_set_t), &mask);
    }
}

struct thread_execution_arguments {
    int process_id;
    int thread_to_test;
};

struct timespec time_measures[2][2];

#define NUMBER_OF_DEBUG_POINTS 3
struct timespec debug_time_measures[2][NUMBER_OF_DEBUG_POINTS];

void *thread_execution(void *data) {
    struct thread_execution_arguments *arguments = (struct thread_execution_arguments *) (data);
    int process_id = (*arguments).process_id;
    int thread_to_test = (*arguments).thread_to_test;

    struct timespec local_debug_time_measures[NUMBER_OF_DEBUG_POINTS];

    // Set max priority for the thread
    // The sched fifo policy avoid involuntary preemption
    struct sched_param my_sched;
    my_sched.sched_priority = sched_get_priority_max(SCHED_FIFO);
    sched_setscheduler(getpid(), SCHED_FIFO, &my_sched);

    // Time measures
    struct timespec start_time, end_time;
    set_affinity_to_cpu(thread_to_test);

    // Now lock all current and future pages from preventing of being paged
    if (mlockall(MCL_CURRENT | MCL_FUTURE))
        perror("mlockall failed");

    // Synchronize both threads
    pthread_barrier_wait(&start_barrier);

    // Debug time get
    clock_gettime(CLOCK_MONOTONIC, &(local_debug_time_measures[0]));
    sched_yield();

    clock_gettime(CLOCK_MONOTONIC, &(local_debug_time_measures[1]));
    sched_yield();

    clock_gettime(CLOCK_MONOTONIC, &(local_debug_time_measures[2]));
    sched_yield();

    // Get actual time in this thread
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // sleep(1); // Force preemption
    sched_yield();

    // For debug purposes get the clock time
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    pthread_barrier_wait(&end_barrier);
    time_measures[process_id][0] = start_time;
    time_measures[process_id][1] = end_time;

    // Unlock pages
    if (munlockall())
        perror("munlockall failed");

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

struct thread_execution_arguments arguments_of_process[2];

int main() {
    int number_of_tests = NUMBER_OF_TESTS;
    pthread_t threads[2];


    if (pthread_barrier_init(&start_barrier, NULL, 2))
        perror("thread barrier initialization failed");

    if (pthread_barrier_init(&end_barrier, NULL, 2))
        perror("thread barrier initialization failed");

    for (int i = 0; i < 2; i++) {
        arguments_of_process[i].process_id = i;
        arguments_of_process[i].thread_to_test = 3;

        if (pthread_create(&threads[i], NULL, &thread_execution, (void *) &(arguments_of_process[i])))
            perror("thread creation failed");
    }

    for (int i = 0; i < 2; i++)
        pthread_join(threads[i], NULL);

    // Calculate differences
    struct timespec result;

    // Preemption time difference
    timespec_subtract(&result, &(time_measures[0][0]), &(time_measures[1][0]));

    long long preemption_cost_nanoseconds = result.tv_sec * 1000000000L + result.tv_nsec;

    printf("Time taken by the preemption %lld \n", preemption_cost_nanoseconds);
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
        perror("bad behaviour of the test");

}
