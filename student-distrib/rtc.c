#include "rtc.h"
#include "lib.h"

// The rtc counter used to count the number of interrupt
static int rtc_counter = 0;
static volatile int rtc_interrupt_occured;
void rtc_set_freq(int rate);

/*
 * rtc_init
 *   DESCRIPTION: Initialize rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: having rtc Initialized
 */
void rtc_init() {
    rtc_interrupt_occured = 0;
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
    rtc_interrupt_occured = 1;
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

/*
 * rtc_close
 *   DESCRIPTION: close rtc file descriptor
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: make it available for later open calls
 */
int32_t rtc_close(int32_t fd) {return 0;}

/*
 * rtc_open
 *   DESCRIPTION: initialize RTC frequency to 2hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_open(const uint8_t* filename) {
    rtc_init();  // initialize RTC, set default frequency to 2 Hz
    rtc_set_freq(RTC_MAX_RATE);
    return 0;
}

/*
 * rtc_write
 *   DESCRIPTION: change the frequency of rtc
 *   INPUTS: buf - pointer to the desired frequency
 *           nbytes - 4
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes) {

    if (nbytes != 4) return -1; // read all the 4 bytes of an int
    if (buf == NULL) return -1;

    int32_t freq = *( (int32_t*) buf);
    if ((freq & (freq - 1)) != 0) return -1; // check is the frequency is the power of 2
    int pow = 0;    // calculate the power
    while (freq > 1) {
        freq = freq >> 1;
        pow++;
    }

    int rate = RTC_MAX_RATE - pow + 1;    // freq = 32768 >> (rate-1)
    if (rate > RTC_MAX_RATE) return -1;
    if (rate < RTC_MIN_RATE) return -1;
    rtc_set_freq(rate);

    return 0;
}

/*
 * rtc_set_freq
 *   DESCRIPTION: helper function, set frequency but no input check
 *   INPUTS: desired rate
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void rtc_set_freq(int rate) {
    cli();
    {
        outb(0x8A, RTC_PORT_0);  //
        char prev = inb(RTC_PORT_1);  //
        outb(0x8A, RTC_PORT_0);  //
        outb((prev & 0xF0) | rate, RTC_PORT_1);  //
    }
    sti();
}

/*
 * rtc_read
 *   DESCRIPTION: block until the next interrupt
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes) {
    rtc_interrupt_occured = 0;
    rtc_restart_interrupt();
    while (!rtc_interrupt_occured) {};
    return 0;
}
