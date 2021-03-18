obj-m += clear_cache_lkm.o
clear_cache_lkm-objs := clear_cache.o clear_cache_module.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=${PWD} modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=${PWD} clean
