# Chromebook Host Kernel
This is the host kernel (3.13) for Chromebook. The nessesary functions to create and set the hardware performance
counters have been added.

## Implementation
To enable the hardware performance counters, the following files have been modified:

arch/arm/kvm/handle_exit.c

arch/arm/kvm/arm.c

zahra/pmu_zahra.c

zahra/pmu_zahra.h

zahra/Makefile

fs/read_write.c

kernel/events/core.c

arch/arm/kernel/perf_event_v7.c

Makefile


## Compile
```shell
ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make uImage dtbs -j4
```
