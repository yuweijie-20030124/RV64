#ifndef __CONFIG_H__
#define __DONFIG_H__

#define CONFIG_RV64         1
#define CONFIG_ISA64        1
#define PMEM64              1

#define CONFIG_TRACE        1
#define CONFIG_TRACE_START  0
#define CONFIG_TRACE_END    10000
#define CONFIG_ITRACE       1
// #define CONFIG_WATCHPOINT   1
#define CONFIG_FTRACE       1
#define CONFIG_MTRACE       1
#define CONFIG_MRACE_START  0
#define CONFIG_MRACE_END    10000
// #define CONFIG_DEVICE       1   // needs device_update() + sdl_clear_event_queue() implementations
#define CONFIG_DIFFTEST     1
// #define CONFIG_VCD_GET      1
#define CONFIG_GET_TIMER    1

// #define CONFIG_ITRACE_COND
// #define ITRACE_COND 1

#endif
