#ifndef __KDEFS_H
#define __KDEFS_H

#define LOG_ON
#ifdef LOG_ON
    #define LOG(...) printk("kernel_agent: " __VA_ARGS__)
#else
    #define LOG(f, ...)
#endif

/*#define LOG_DEBUG_ON*/
#ifdef LOG_DEBUG_ON
    #define LOGD(...) printk("vmm_agent: " __VA_ARGS__)
#else
    #define LOGD(f, ...)
#endif

#endif
