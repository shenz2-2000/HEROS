#include "rtc.h"
#include "lib.h"
#include "cmos.h"
// The rtc counter used to count the number of interrupt
// static int fake_interval = 1;
// static volatile int rtc_interrupt_occured;
void rtc_set_freq(int rate);
static int virtual_ctr[] = {-1, -1, -1, -1, -1, 0};
static volatile int ticks[] = {0, 0, 0, 0, 0, 0};
void system_time();
/*
 * rtc_init
 *   DESCRIPTION: Initialize rtc
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: having rtc Initialized
 */
void rtc_init() {
    // rtc_interrupt_occured = 0;
    //cli();
    outb(0x8B, RTC_PORT_0); // Select register B, and disable NMI
    char prev = inb(RTC_PORT_1); // Read current value of register B
    outb(0x8B, RTC_PORT_0); // Set the index again
    outb(prev|0x40, RTC_PORT_1); // write the previous value ORed with 0x40. This turns on bit 6 of register B
    //sti();

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
    // system_time();
    // rtc_interrupt_occured = 1;
    ticks[0] = 1;
    ticks[1] = 1;
    ticks[2] = 1;
    ticks[3] = 1;
    ticks[4] = 1;
    ticks[5] = 1;
    rtc_restart_interrupt();
    sti();
    show_screen();
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
 * rtc_close
 *   DESCRIPTION: close rtc file descriptor
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: make it available for later open calls
 */
int32_t rtc_close(int32_t fd) {
    pcb_t *cur_pcb = get_cur_process();
    virtual_ctr[(int) cur_pcb->rtc_id] = -1;
    cur_pcb->rtc_id = -1;
    return 0;
}

/*
 * rtc_open
 *   DESCRIPTION: initialize RTC frequency to 2hz
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: 0 on success and -1 on failure
 *   SIDE EFFECTS: none
 */
int32_t rtc_open(const uint8_t* filename) {
    pcb_t *cur_pcb = get_cur_process();
    int i;
    if (cur_pcb->rtc_id == -1) {
        for (i = 0; i < MAX_CLK; ++i) {
            if (virtual_ctr[i] == -1) break;
        }
        if (i == MAX_CLK) {
            printf("No available virtual RTC. RTC open failed.\n");
            return -1;
        }
        cur_pcb->rtc_id = i;
    }
    virtual_ctr[(int) cur_pcb->rtc_id] = 1;
    rtc_init();  // initialize RTC, set default frequency to 1024 Hz
    rtc_set_freq(8);
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
    if (rate <= RTC_MIN_RATE) return -1;

    pcb_t *cur_pcb = get_cur_process();
    virtual_ctr[(int) cur_pcb->rtc_id] = 1024 >> (pow+2);

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
    int rid = (int) get_cur_process()->rtc_id;
    int ctr = virtual_ctr[rid];
    while (ctr > 0) {
        ticks[rid] = 0;
        while (!ticks[rid]) {};
        ctr--;
    }
    return 0;
}

/* sleep
 * Description: it sleep for roughly a certain time
 * Inputs: time_in_ms - duration in ms unit
 * Return Value: ret: 0 if success, -1 if fail
 */
int32_t sleep(uint32_t time_in_ms) {
    int32_t fd, i;
    double approx;

    if (time_in_ms == 0) return 0;

    // manually adapt the time
    /*
     * NOTE: because of all kinds of delays and errors, the actual delayed time is not
     * really the given one. This magic number 0.7 is a experimental value to fit this
     * error. The range of this approximation is about 0-15 seconds (for more than 15 
     * seconds, the real time delayed is shorter than the value given (even not linear)).
     */
    approx = (double) time_in_ms * 0.7;
    time_in_ms = (uint32_t) approx;

    // open an rtc file
    fd = open((uint8_t*) "rtc");
    if (fd == -1) return -1;
    // set the frequence to 1024 (close to 1000 and is the max freq)
    rtc_set_freq(RTC_MIN_RATE);
    // loop
    for (i = 0; i < time_in_ms; ++i) {
        ticks[5] = 0;
        rtc_restart_interrupt();
        while(!ticks[5]) {};
    }
    close(fd);
    return 0;
}

