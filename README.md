# Preemption and migration cost analysis tools

## Results for Raspberry Pi 3B
Kernel: 4.19.0-14-rt-arm64  
OS: Debian  

### Preemption results
Number of experiments: 100  
Minimum cost of preemption: 7760 ns  
Maximum cost of preemption: 11250 ns  
Average cost of preemption: 8034 ns

### Migration result
Number of experiments: 100  
Minimum cost of migration: 31875 ns  
Maximum cost of migration: 56042 ns  
Average cost of migration: 33058 ns

### L2 cache fill cost (this experiment isn't working as expected)
Experiment result:
Number of experiments: 100
Minimum cost of fill half l2 cache: -15209 ns
Maximum cost of fill half l2 cache: 15052 ns
Average cost of fill half l2 cache: -69 ns