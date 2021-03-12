# Overhead analysis

This repo contains some benchmarks to analyze the cost produced by the following actions:

- Preempt a process
- Migrate a process
- Fill the L2 cache memory from main memory

## Environment

The analysis have been done in a Raspberry Pi 3B with Debian and the Linux kernel version 4.19.0-14-rt-arm64.

## Preemption analysis

The preemption analysis benchmark is found in the [preemption_cost](./preemption_cost) folder.

This benchmark do the following:

- lock all used memory to avoid page faults
- create two threads, booth in the same CPU with priority RT and sched type FIFO to avoid involuntary preemption
- the cost measured is the time taken from an invocation of the __sched_yield__ function(voluntary preemption) in one
  thread until the other thread starts its execution

### Results for 100 experiments

| Minimum cost | Maximum cost | Average cost |
|--------------|--------------|--------------|
| 7760 ns      | 11250 ns     | 8034 ns      |

## Migration analysis

The migration analysis benchmark is found in the [migration_cost](./migration_cost) folder.

This benchmark do the following:

- lock all used memory to avoid page faults
- set priority RT and sched type FIFO
- set affinity to an unused core
- the cost measured is the time taken from an invocation of the __sched_setaffinity__ function, setting affinity to
  other unused core, until the process start the execution in the other core

### Results for 100 experiments

| Minimum cost | Maximum cost | Average cost |
|--------------|--------------|--------------|
| 31875 ns     | 56042 ns     | 33058 ns     |

## L2 cache fill cost analysis

The L2 cache fill cost analysis benchmark is found in the [cache_management](./cache_management) folder.

This benchmark do the following:

- lock all used memory to avoid page faults
- set priority RT and sched type FIFO
- set affinity to an unused core
- iterate over a vector (with the size of half of the L2 cache) loading each element to a register to fill l2 cache
- measure the cost of iterating over the same vector with it loaded in the L2 cache
- flush the cache
- measure the cost of iterating over the same vector without having it loaded in the L2 cache
- the measured cost is the difference between the cost when the vector is not loaded and when it is loaded

### Results for 100 experiments

Warning: This experiment isn't working as expected

| Minimum cost | Maximum cost | Average cost |
|--------------|--------------|--------------|
| -15209 ns    | 15052 ns     | -69 ns       |
