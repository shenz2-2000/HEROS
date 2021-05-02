// reference: http://homepages.cae.wisc.edu/~brodskye/sb16doc/sb16doc.html

/*
 * TODO:
 * 1. [-] initialize the mem area of DSP page (in paging.c)
 */

#include "sound_card.h"
#include "lib.h"
#include "i8259.h"

static int32_t dsp_status = DSP_OFF;

int32_t dsp_reset() {
    int i = 0;
    uint8_t ret;

    if (dsp_status == DSP_ON) return -1;

    cli();
    outb(1, DSP_RESET_PORT);
    for (i = 0; i < 15000; i++);    // wait for 3 microseconds
    outb(0, DSP_RESET_PORT);
    ret = inb(DSP_READ_PORT);
    
    // if  the DSP read port return not 0xAA, the reset fail
    if (ret != DSP_STATUS_READY) {
        sti();
        return -1;
    }

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

    // initialize end
    sti();

    return 0;
}
