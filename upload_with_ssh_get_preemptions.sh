#!/bin/bash
ARCHITECTURE=aarch64

scp ./builds/${ARCHITECTURE}/{get_preemption_time_linux_o0,get_preemption_time_linux_o1,get_preemption_time_linux_o2,get_preemption_time_linux_o3} root@192.168.2.2:/root/
