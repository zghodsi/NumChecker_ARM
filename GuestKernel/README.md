# Chromebook Guest Kernel
This is the guest kernel (3.9) for Chromebook. A variable is added (counter_on_off) to control when we want the system
call to be monitored. It is set by a new system call (sys_zahra) added to the kernel. This system call should be envoked
before envoking the system call that we want to monitor.


## Implementation
To enable monitoring of the system calls, the following files have been modified:

- arch/arm/kernel/sys_arm.c

- arch/arm/kernel/Calls.s

- include/linux/syscalls.h

- fs/read_write.c

- fs/readdir.c

- fs/ioctl.c

## Compile

To compile and append the dtb file run:
```shell
ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- make zImage dtbs -j4
cat arch/arm/boot/dts/rtsm_ve-cortex_a15x1.dtb >> arch/arm/boot/zImage
```
