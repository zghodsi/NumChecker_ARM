#include <linux/kernel.h>
#include <linux/time.h> 
#include <linux/perf_event.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/fcntl.h>
#include <linux/file.h>
#include <linux/uio.h>
#include <linux/fsnotify.h>
#include <linux/security.h>
#include <linux/module.h>
#include <linux/pagemap.h>
#include <linux/splice.h>
#include <asm/uaccess.h>
#include <asm/unistd.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#define CPU_CYCLES                 0
#define INSTRUCTIONS               1
#define CACHE_REFERENCES           2
#define CACHE_MISSES               3
#define BRANCH_INSTRUCTIONS        4
#define BRANCH_MISSES              5
#define BUS_CYCLES                 6
#define STALLED_CYCLES_FRONTEND    7
#define STALLED_CYCLES_BACKEND     8
 
//raw events
#define RETURN_BRANCH              0x8F
#define DIVIDE_OP                  0x13
#define MULTILY_OP                 0x12
#define FP_MICRO_OP                0x10

typedef unsigned long long u64;  
typedef signed long long   s64;  
typedef unsigned int       u32;  
typedef signed int     s32;  
typedef unsigned short     u16;  
typedef signed short       s16;  
typedef unsigned char      u8;  
typedef signed char    s8;  

void disable_counter(int);
void enable_counter(int);
void reset_counter(int);
int create_counter_sample(int, int, long long int);
int create_counter(int, int, long long int);
u64 read_counter(int); 
//int init_perf(int, int, long long int); 
