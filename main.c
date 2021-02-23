#define _GNU_SOURCE

#include <stdio.h>
#include <sys/resource.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/wait.h>

/**
 * Information about usage
 */
struct usage_statics {
    long user_time_us, user_time_s, system_time_us, system_time_s, involuntary_context_switches, voluntary_context_switches;
    double elapsed_clock;
};

/**
 * Get the actual usage statics
 * @param measures
 * @return
 */
int get_actual_usage_statics(struct usage_statics *measures) {
    struct rusage usage;
    int success = getrusage(RUSAGE_SELF, &usage);
    if (success == 0) {
        measures->user_time_us = usage.ru_utime.tv_usec;
        measures->user_time_s = usage.ru_utime.tv_sec;
        measures->system_time_us = usage.ru_stime.tv_usec;
        measures->system_time_s = usage.ru_stime.tv_sec;
        measures->involuntary_context_switches = usage.ru_nivcsw;
        measures->voluntary_context_switches = usage.ru_nvcsw;
        return 0;
    } else {
        return -1;
    }
}

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

void usage_statistics_difference_print(struct usage_statics initial_measures, struct usage_statics final_measures) {
    long user_time_us = final_measures.user_time_us - initial_measures.user_time_us;
    long user_time_s = final_measures.user_time_s - initial_measures.user_time_s;

    if (user_time_us < 0) {
        user_time_s = user_time_s - 1;
        user_time_us = user_time_us + 1000000L;
    }

    long voluntary_context_switches =
            final_measures.voluntary_context_switches - initial_measures.voluntary_context_switches;
    long involuntary_context_switches =
            final_measures.involuntary_context_switches - initial_measures.involuntary_context_switches;

    printf("%ld, %ld, %ld, %ld\n",
           user_time_s, user_time_us,
           voluntary_context_switches, involuntary_context_switches);
}

#define NUMBER_OF_ITERATIONS 1000000000
#define NUMBER_OF_MIGRATIONS 10

volatile float external_variable;

int main_continuous() {
#ifdef PRINT_EXECUTION_TIME
    struct usage_statics initial_measures, final_measures;
#endif
    // Set priority and start timers
    set_affinity_to_cpu(2);

    struct sched_param my_sched;
    my_sched.sched_priority = sched_get_priority_max(SCHED_FIFO);

    sched_setscheduler(getpid(), SCHED_FIFO, &my_sched);

#ifdef PRINT_EXECUTION_TIME
    get_actual_usage_statics(&initial_measures);
#endif
    int pid = fork();

    // Compute intensive code
    external_variable = 14.0f;
    for (int i = 0; i < NUMBER_OF_ITERATIONS; ++i) {
        external_variable = 3.186f * external_variable * external_variable * external_variable * external_variable +
                            external_variable;
        external_variable = 14.0f;
    }

    if (pid != 0) {
        // Parent process
        wait(NULL);
#ifdef PRINT_EXECUTION_TIME
        // Get results
        get_actual_usage_statics(&final_measures);
        usage_statistics_difference_print(initial_measures, final_measures);
#endif
    }

    return 0;
}

int main_migration() {
#ifdef PRINT_EXECUTION_TIME
    struct usage_statics initial_measures, final_measures;
#endif

    int pid = fork();

    int actual_core;
    // Set priority and start timers
    if (pid == 0) {
        actual_core = 0;
    } else {
        actual_core = 1;
#ifdef PRINT_EXECUTION_TIME
        get_actual_usage_statics(&initial_measures);
#endif
    }

    set_affinity_to_cpu(2 + actual_core);

    struct sched_param my_sched;
    my_sched.sched_priority = sched_get_priority_max(SCHED_RR);

    sched_setscheduler(getpid(), SCHED_RR, &my_sched);

    // Compute intensive code
    external_variable = 14.0f;
    for (int j = 0; j < NUMBER_OF_MIGRATIONS; ++j) {
        for (int i = 0; i < NUMBER_OF_ITERATIONS / NUMBER_OF_MIGRATIONS; ++i) {
            external_variable = 3.186f * external_variable * external_variable * external_variable * external_variable +
                                external_variable;
            external_variable = 14.0f;
        }
        actual_core = 1 - actual_core;
        set_affinity_to_cpu(2 + actual_core);
    }

    if (pid != 0) {
        // Parent process
        wait(NULL);

#ifdef PRINT_EXECUTION_TIME
        // Get results
        get_actual_usage_statics(&final_measures);
        usage_statistics_difference_print(initial_measures, final_measures);
#endif
    }

    return 0;
}

int main_preemption() {
#ifdef PRINT_EXECUTION_TIME
    struct usage_statics initial_measures, final_measures;
#endif
    // Set priority and start timers
    set_affinity_to_cpu(2);

    struct sched_param my_sched;
    my_sched.sched_priority = sched_get_priority_max(SCHED_RR);

    sched_setscheduler(getpid(), SCHED_RR, &my_sched);

#ifdef PRINT_EXECUTION_TIME
    get_actual_usage_statics(&initial_measures);
#endif

    int pid = fork();

    // Compute intensive code
    external_variable = 14.0f;
    for (int i = 0; i < NUMBER_OF_ITERATIONS; ++i) {
        external_variable = 3.186f * external_variable * external_variable * external_variable * external_variable +
                            external_variable;
        external_variable = 14.0f;
    }

    if (pid != 0) {
        // Parent process
        wait(NULL);
#ifdef PRINT_EXECUTION_TIME
        // Get results
        get_actual_usage_statics(&final_measures);
        usage_statistics_difference_print(initial_measures, final_measures);
#endif
    }
    return 0;
}

int main() {
#ifdef ANALIZE_PREEMPTION
    return main_preemption();
#endif

#ifdef ANALIZE_CONTINUOUS
    return main_continuous();
#endif

#ifdef ANALIZE_MIGRATION
    return main_migration();
#endif
}
