# HPC_Chromebook_HostKernel
This is the host kernel (3.13) for Chromebook. The nessesary functions to create and set the hardware performance
counters have been added.


Files modified:

arch/arm/kvm/handle_exit.c

arch/arm/kvm/arm.c

zahra/pmu_zahra.c

zahra/pmu_zahra.h

zahra/Makefile

fs/read_write.c

kernel/events/core.c

arch/arm/kernel/perf_event_v7.c

Makefile


To compile:

ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make uImage dtbs -j4
