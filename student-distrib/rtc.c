#include "rtc.h"
#include "lib.h"

// The rtc counter used to count the number of interrupt
static int rtc_counter = 0;

/*
 * rtc_init
 *   DESCRIPTION: Initialize rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: having rtc Initialized
 */
void rtc_init() {
    cli();
    outb(0x8B, RTC_PORT_0); // Select register B, and disable NMI
    char prev = inb(RTC_PORT_1); // Read current value of register B
    outb(0x8B, RTC_PORT_0); // Set the index again
    outb(prev|0x40, RTC_PORT_1); // write the previous value ORed with 0x40. This turns on bit 6 of register B
    sti();

    //Reference: https://wiki.osdev.org/RTC
}

/*
 * rtc_interrupt_handler
 *   DESCRIPTION: Handle the interrupt from rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void rtc_interrupt_handler() {
    cli();
    ++rtc_counter;
    // Virtualize the RTC and change the frequency
    if (rtc_counter>=RTC_LIMIT){
//        printf("RECEIVE %d RTC Interrupts\n", rtc_counter);
        rtc_counter = 0;
    }
    // Restart so it can send interrupt again
    rtc_restart_interrupt();
    sti();
    //test_interrupts();
}

/*
 * rtc_restart_interrupt
 *   DESCRIPTION: restart the interrupt of rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void rtc_restart_interrupt(){
    outb(0x8C, RTC_PORT_0);
    inb(RTC_PORT_1);
}

/*
 * get_rtc_counter
 *   DESCRIPTION: Return the rtc_counter when external request
 *   INPUTS: none
 *   OUTPUTS: rtc_counter
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
// TODO: delete this function after testing
extern int get_rtc_counter(){
    return rtc_counter;
}
