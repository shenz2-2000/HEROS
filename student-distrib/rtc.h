#ifndef RTC_H
#define RTC_H

#define RTC_PORT_0    0x70
#define RTC_PORT_1    0x71
#define RTC_LIMIT     10000

void rtc_interrupt_handler();
void rtc_init();
void rtc_restart_interrupt();

#endif