#!/bin/bash
# No print version
scp ./builds/{main_continuous,main_migration,main_preemption} root@192.168.2.2:/root/

# Print version
scp ./builds/{main_continuous_print,main_migration_print,main_preemption_print} root@192.168.2.2:/root/
