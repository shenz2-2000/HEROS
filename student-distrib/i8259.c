/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/*
 * i8259_init
 * This function initialize the 8259 PIC
 * Input: None.
 * Output: None.
 * Side effect: initialize the 8259 PIC.
 */
void i8259_init(void) {

    // lock ???

    outb(0xFF, MASTER_DATA_PORT);           /* mask all of master */
    outb(0xFF, SLAVE_DATA_PORT);            /* mask all of slave */

    /* init master */
    outb(ICW1, MASTER_CMD_PORT);            /* select master init */
    outb(ICW2_MASTER, MASTER_DATA_PORT);    /* IR0-7 mapped to 0x20-0x27 */
    outb(ICW3_MASTER, MASTER_DATA_PORT);    /* the master has a slave on IR2 */
    outb(ICW4, MASTER_DATA_PORT);           /* master expects normal eoi */

    /* init slave */
    outb(ICW1, SLAVE_CMD_PORT);             /* select slave init */
    outb(ICW2_SLAVE, SLAVE_DATA_PORT);      /* IR0-7 mapped to 0x20-0x27 */
    outb(ICW3_SLAVE, SLAVE_DATA_PORT);      /* the slave has a slave on IR2 */
    outb(ICW4, SLAVE_DATA_PORT);            /* slave's support for AEOI in flat */
                                            /* mode is to be investigated*/

    // udelay(100);                         /* wait for PIC to initialize */

    /* disable interrupts (mask all interrupts at first) */
    master_mask = 0xFF;
    slave_mask = 0xFF;
    outb(master_mask, MASTER_DATA_PORT);    /* restore master IRQ mask */
    outb(slave_mask, SLAVE_DATA_PORT);      /* restore slave IRQ mask */

    // enable slave cascade port, which is 0x02
    enable_irq(SLAVE_CAS_PORT);

    // lock ???

}

/* ref: https://wiki.osdev.org/PIC */
/*
 * enable_irq
 * This enable (unmask) the specified IRQ
 * Input: uint32_t irq_num -- the target irq number.
 * Output: None.
 * Side effect: enable (unmask) the specified IRQ.
 */

void enable_irq(uint32_t irq_num) {
    if (irq_num >= 2 * I8259_N_PORTS) return;

    if (irq_num >= I8259_N_PORTS) { /* enable slave port */
        // merge the new mask into the old mask
        slave_mask &= ~(0x01 << (irq_num - I8259_N_PORTS)); // clear the ith bit
        outb(slave_mask, SLAVE_DATA_PORT);
        // also enable the acscaded port of slave, which is 0x02
        master_mask &= ~(0x01 << SLAVE_CAS_PORT);
        outb(master_mask, MASTER_DATA_PORT);
    } else {                        /* enable master port */
        master_mask &= ~(0x01 << irq_num);
        outb(master_mask, MASTER_DATA_PORT);
    }
}

/*
 * disable_irq
 * This function disable (mask) the specified IRQ
 * Input: uint32_t irq_num -- the target irq number.
 * Output: None.
 * Side effect: disable (mask) the specified IRQ.
 */

void disable_irq(uint32_t irq_num) {
    if (irq_num >= 2 * I8259_N_PORTS) return;

    if (irq_num >= I8259_N_PORTS) { /* disable slave port */
        // merge the new mask into the old mask
        slave_mask |= (0x01 << (irq_num - I8259_N_PORTS)); // clear the ith bit
        outb(slave_mask, SLAVE_DATA_PORT);
    } else {                        /* disable master port */
        master_mask |= (0x01 << irq_num);
        outb(master_mask, MASTER_DATA_PORT);
    }
}

/*
 * send_eoi
 * This function Send end-of-interrupt signal for the specified IRQ
 * Input: uint32_t irq_num -- the target irq number.
 * Output: None.
 * Side effect: send end-of-interrupt signal for the specified IRQ.
 */

void send_eoi(uint32_t irq_num) {
    if (irq_num >= 2 * I8259_N_PORTS) return;
    
    if (irq_num >= I8259_N_PORTS) {
        // send the EOI to slave
        outb((uint8_t)(EOI | (irq_num - I8259_N_PORTS)), 
             SLAVE_CMD_PORT);
        // send the EOI to master (slave cascaded port)
        outb((uint8_t)(EOI | SLAVE_CAS_PORT),
             MASTER_CMD_PORT);
    } else {
        // send the EOI to master
        outb((uint8_t)(EOI | irq_num),
             MASTER_CMD_PORT);
    }
}
