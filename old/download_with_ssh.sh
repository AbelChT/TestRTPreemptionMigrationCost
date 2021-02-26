#!/bin/bash

scp root@192.168.2.2:/root/{continuous_function_trace.dat,migration_function_trace.dat,preemption_function_trace.dat,continuous_function_graph_trace.dat,migration_function_graph_trace.dat,preemption_function_graph_trace.dat} ./traces-preempt-rt/function_without_irq_isolation/
