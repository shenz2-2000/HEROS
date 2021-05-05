#include "types.h"
#include "process.h"
#include "vga.h"

#ifndef RTC_H
#define RTC_H

#define RTC_PORT_0    0x70
#define RTC_PORT_1    0x71
#define RTC_LIMIT     10000
/* freq = 32768 >> (rate-1) */
#define RTC_MIN_RATE  6     // 1khz
#define RTC_MAX_RATE  15    // 2hz
#define MAX_CLK 6

void rtc_interrupt_handler();
void rtc_init();
void rtc_restart_interrupt();

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t rtc_open(const uint8_t* filename);
int32_t rtc_close(int32_t fd);
extern int all_terminal_is_on;
/* utils */
int32_t sleep(uint32_t time_in_ms);

#endif
