#ifndef __DEVICE_H__
#define __DEVICE_H__

#include "common.h"

#define DEVICE_ADDR     0xa0000000

#ifndef CONFIG_SERIAL_MMIO
#define CONFIG_SERIAL_MMIO 0xa0000000
#endif

#ifndef CONFIG_RTC_MMIO
#define CONFIG_RTC_MMIO 0xa0000100
#endif

#ifndef CONFIG_SBI_DISK_MMIO
#define CONFIG_SBI_DISK_MMIO 0x10100000
#endif

#ifndef CONFIG_SBI_SERIAL_MMIO
#define CONFIG_SBI_SERIAL_MMIO 0x10000000
#endif

#ifndef CONFIG_SBI_CLINT_MMIO
#define CONFIG_SBI_CLINT_MMIO 0x2000000
#endif

#ifndef CONFIG_SBI_PLIC_MMIO
#define CONFIG_SBI_PLIC_MMIO 0xc000000
#endif

#ifndef CONFIG_SBI_PLIC_CONTEXT_COUNT
#define CONFIG_SBI_PLIC_CONTEXT_COUNT 2
#endif

#define SERIAL_ADDR     CONFIG_SERIAL_MMIO
#define TIMER_ADDR      CONFIG_RTC_MMIO

void serial_out(char ch);
void get_rtc();
void get_uptime();
uint32_t get_timer_reg(int offest);
void sbi_serial_io_handler_r(uint64_t raddr, uint64_t *rdata);
void sbi_serial_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_disk_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_disk_io_handler_r(uint64_t raddr, uint64_t *rdata);
void sbi_clint_io_handler_r(uint64_t raddr, uint64_t *rdata);
void sbi_clint_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_plic_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask);
void sbi_plic_io_handler_r(uint64_t raddr, uint64_t *rdata);
void device_update(void);
void sdl_clear_event_queue(void);

extern void set_skip_ref_flag(void);

#endif
