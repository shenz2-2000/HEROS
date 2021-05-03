// reference: http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html

/*
 * TODO:
 * 1. [-] initialize the mem area of DSP page (in paging.c)
 * 2. [-] analyze the wav file
 */

#include "sound_card.h"
#include "lib.h"
#include "i8259.h"
#include "link.h"

static int32_t dsp_status = DSP_OFF;

// to indicate an interrupt, we need a counter to track the interrupt.
volatile uint32_t dsp_int_cnt = 0;


/**
 * audio_open
 * Description: init the sound card
 * Input: None
 * Output: 0 if success, -1 if not
 */
int32_t audio_open() {
    int i = 0;
    uint8_t ret;

    if (dsp_status == DSP_ON) return -1;

    cli();
    outb(1, DSP_RESET_PORT);
    for (i = 0; i < 15000; i++);    // wait for 3 microseconds
    outb(0, DSP_RESET_PORT);
    ret = inb(DSP_READ_PORT);
    
    // if  the DSP read port return not 0xAA, the reset fail
    if (ret != DSP_STATUS_READY) { sti(); return -1; }

    dsp_status = DSP_ON;
    enable_irq(DSP_IRQ_NUM);    // the interrupt number is 5

    /* programming DMA */
    // FIXME: maybe the order is wrong
    // Disable channel 1
    outb(DMA_CH_1 + DMA_CH_DISABLE, DMA_REG_MASK);
    // write value to flip-flop port 0x0C (any value) to reset flip-flop
    outb(1, DMA_REG_CLEAR_FF);
    // send transfer mode to 0x0B (0x48 for single mode/0x58 for auto mode + channel number)
    outb(DMA_MODE_AUTO_PB, DMA_REG_MODE);
    // send page number to 0x83
    outb(DSP_BUF_ADDR >> 16, DMA_CH_1_PAGE_PORT);
    // send low bits of position
    outb((uint8_t) (DSP_BUF_ADDR), DMA_CH_1_ADDR_PORT);
    // send high bits of position
    outb((uint8_t) (DSP_BUF_ADDR >> 8), DMA_CH_1_ADDR_PORT);
    // send low bits of length of data
    outb((uint8_t) (DSP_BUF_LEN - 1), DMA_CH_1_CNT_PORT);
    // send high bits of length of data
    outb((uint8_t) ((DSP_BUF_LEN - 1) >> 8), DMA_CH_1_CNT_PORT);
    // enable channel 1
    outb(DMA_CH_1 + DMA_CH_ENABLE, DMA_REG_MASK);

    // initialization end
    sti();

    return 0;
}

/**
 * audio_close
 * Description: free the sound blaster 16 control
 * Input: None
 * Output: 0 if success, -1 if not
 */
int32_t audio_close() {
    if (dsp_status == DSP_OFF) {
        printf("WARNING in audio_close: cannot close an inactive sound card.\n");
        return -1;
    }
    dsp_status = DSP_OFF;
    return 0;
}

/**
 * audio_read
 * Description: wait an interrupt and return
 * Input: None
 * Output: 0 if success, -1 if not
 */
int32_t audio_read() {
    uint32_t temp;
    if (dsp_status == DSP_OFF) {
        printf("WARNING in audio_close: cannot read an inactive sound card.\n");
        return -1;
    }
    // keep track of the interrupt (return if interrupted)
    temp = dsp_int_cnt;
    while (1) if (dsp_int_cnt != temp) break;
    return 0;
}

/**
 * audio_write
 * Description: write the audio data into the dsp buffer
 * Input: pos - the position of the current chunk in the dsp buffer (dst)
          buf - the buffer (src)
          bufsize - the number of bytes of a buffer
 * Output: the length of copy of this activity
 */
int32_t audio_write(uint32_t *pos, const void *buf, int32_t bufsize) {
    uint32_t cutoff;
    int32_t ret;
    if (dsp_status == DSP_OFF) {
        printf("WARNING in audio_write: cannot write with an inactive sound card.\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR in audio_write: input buf NULL pointer.\n");
        return -1;
    }
    if (*pos + bufsize > DSP_BUF_LEN) {
        cutoff = DSP_BUF_LEN - *pos;
        memcpy((uint8_t*) (DSP_BUF_ADDR + *pos), buf, bufsize);
        *pos = 0;
        ret = audio_write(pos, (uint8_t*) buf + cutoff, bufsize - cutoff);
        if (ret != bufsize - cutoff) return -1;
    } else {
        memcpy((uint8_t*) (DSP_BUF_ADDR + *pos), buf, bufsize);
        *pos = *pos + bufsize;
    }
    return bufsize;
}

/**
 * audio_ioctl
 * Description: io control - send commend to the device
 * Input: cmd - the command
 * Output: 0 if success
 */
int32_t audio_ioctl(uint8_t cmd) {
    if (dsp_status == DSP_OFF) {
        printf("WARNING in audio_ioctl: the sound card is inactive.\n");
        return -1;
    }
    outb(cmd, DSP_WRITE_PORT);
    return 0;
}

/**
 * dsp_interrupt_handler
 * Description: DSP interrupt handler
 * Input: None
 * Output: None
 */
ASMLINKAGE void dsp_interrupt_handler() {
    dsp_int_cnt++;
    inb(DSP_READ_STATUS_PORT);
    send_eoi(DSP_IRQ_NUM);
}
