#include "scheduler.h"
#include "lib.h"
#include "idt.h"

static void setup_pit(uint16_t hz);

static int idt_test_cnt = 0;


/**
 * initialize the scheduler
 */
void init_scheduler() {
    setup_pit(INIT_PIT_FREQ);
}


/**
 * Interrupt handler of PIT
 * @usage in boot.S
 */
ASMLINKAGE void pit_interrupt_handler() {
    idt_test_cnt++;

//    if (idt_test_cnt % 100 == 0)
//    {
//        puts("Tick;\n");
//    }
}


/**
 * set up PIT frequency
 * @param hz   desired PIT frequency
 * Reference: http://www.osdever.net/bkerndev/Docs/pit.htm
 */
static void setup_pit(uint16_t hz) {
    uint16_t divisor = 1193180 / hz;    /* Calculate our divisor */
    outb(0x36, 0x34);  // set command byte 0x36
    outb(divisor & 0xFF, 0x40);   // Set low byte of divisor
    outb(divisor >> 8, 0x40);     // Set high byte of divisor
}
