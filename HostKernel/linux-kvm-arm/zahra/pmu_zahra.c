#include "pmu_zahra.h"

extern long perf_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
//static struct perf_event_attr attrs[]={};
static struct perf_event_attr attrs[] = {  
  { .type = PERF_TYPE_HARDWARE, .config = PERF_COUNT_HW_INSTRUCTIONS}  
}; 
extern ssize_t terry_read(unsigned int fd, char * buf, size_t count);
int terry_perf_open(
		struct perf_event_attr *attr_in,
		pid_t pid, int cpu, int group_fd, unsigned long flags);

void disable_counter(int fd)
{

    int fput_needed;

    struct file *file;
 

    file = fget_light(fd, &fput_needed);
    perf_ioctl(file, PERF_EVENT_IOC_DISABLE,0);
    fput_light(file, fput_needed);

}
//-----------------------enable counters------------------------------
void enable_counter(int fd)
{

    int fput_needed;
    struct file *file;
 

    file = fget_light(fd, &fput_needed);
    perf_ioctl(file, PERF_EVENT_IOC_ENABLE,0);
    fput_light(file, fput_needed);

}
//-----------------------reset counters-------------------------------
void reset_counter(int fd)
{

    int fput_needed;
    struct file *file;
 
    file = fget_light(fd, &fput_needed);
    perf_ioctl(file, PERF_EVENT_IOC_RESET,0);
    fput_light(file, fput_needed);

} 
//-----------------create_counter---------------------------------------
int create_counter_sample(int event_type, int pid, long long int event)
{
    	struct perf_event_attr attr;
        int fd;
 
        memcpy(&attr, attrs, sizeof(struct perf_event_attr)); 
             
		attr.sample_period   = 100000;
	        attr.type            = event_type;
                attr.config          = event;
                attr.inherit         = 1; 
                attr.disabled        = 1; 
                attr.enable_on_exec  = 0; 
                attr.size = sizeof(attr);
 
    	fd = terry_perf_open(&attr, pid, -1, -1, 0);
 
        return fd;
}

int create_counter(int event_type, int pid, long long int event)
{
    	struct perf_event_attr attr;
        int fd;
 
        memcpy(&attr, attrs, sizeof(struct perf_event_attr)); 
             
//		attr.sample_period   = 10000;

	        attr.type            = event_type;
                attr.config          = event;
                attr.inherit         = 1; 
                attr.disabled        = 1; 
                attr.enable_on_exec  = 0; 
                attr.size = sizeof(attr);
                
 
    	fd = terry_perf_open(&attr, pid, -1, -1, 0);
    	printk(KERN_DEBUG "fd= %d\n",fd); 
    	

        return fd;
}

u64 read_counter(int fd) 
{ 
    u64 single_count[3]; 
    size_t res; 
    u64 read_val=0;
 
    if (fd <= 0) 
        return 0; 
    res = terry_read(fd, single_count, sizeof(u64)); 
    if(res == sizeof(u64)) { 
         read_val=single_count[0]; 
         //printk(KERN_ALERT "count 1: %lld\n", read_val);
    }else { 
         printk(KERN_ALERT "Fail to read counter.\n"); 
         printk(KERN_ALERT "res: %d\n", res);
    } 
   return read_val;
}

