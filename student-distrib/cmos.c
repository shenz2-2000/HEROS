//
// Created by 10656 on 2021/5/2.
//

#include "cmos.h"
#include "rtc.h"
#include "lib.h"
#include "terminal.h"
#include "gui.h"
/*The following parts are concerning CMOS system time*/
// Reference Comes from: https://wiki.osdev.org/CMOS#Getting_Current_Date_and_Time_from_RTC\

int32_t sec, mins, hour, day, month, year;
#define CMOS_address 0x70
#define CMOS_data 0x71
int waiting_for_ack(){
    outb(0x0A, CMOS_address);
    int input = inb(CMOS_data);
    return input&0x80;
}

unsigned get_data_from_register(int reg) {
    outb(reg, CMOS_address);
    return inb(CMOS_data);
}

void system_time(){
    while (waiting_for_ack());
    sec = get_data_from_register(0x00);
    mins = get_data_from_register(0x02);
    hour = get_data_from_register(0x04);
    day = get_data_from_register(0x07);
    month = get_data_from_register(0x08);
    year = get_data_from_register(0x09);

    int Mode = get_data_from_register(0x0B);
    if (!(Mode&0x04)) {
        hour = (hour >> 4) * 10 + (hour & 0x0f);
        mins = (mins >> 4) * 10 + (mins & 0x0f);
        sec = (sec >> 4) * 10 + (sec & 0x0f);
        year = (year >> 4) * 10 + (year & 0x0f);
        month = (month >> 4) * 10 + (month & 0x0f);
        day = (day >> 4) * 10 + (day & 0x0f);
    }

    if (!(Mode&0x02) && (hour&0x80)) hour = ((hour & 0x7F) + 12) % 24;
//    uint32_t flags;
//    cli_and_save(flags);
//    terminal_struct_t* running_terminal = get_running_terminal();
//    if (get_showing_task())
//        terminal_set_running(get_showing_task()->terminal);
//    // printf("UTC+0: 2021/%d/%d %d:%d:%d\n", month, day, hour, mins, sec);
//    terminal_set_running(running_terminal);
    setup_status_bar();
//    restore_flags(flags);
}