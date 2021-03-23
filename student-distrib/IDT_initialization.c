//
// Created by BOURNE on 2021/3/23.
//
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"

/* Declaration of constant in interrupt */

#define IDT_ENTRY_KEYBOARD 0x21
#define IDT_ENTRY_RTC 0x28
#define IDT_SYSTEM_CALL 0x80


/*
 * init_IDT
 * This function will initialize the IDT with dummy handlers
 * Input: None
 * Output: None.
 * Side effect: Fill in the IDT
 */
void init_IDT(){
    int i;

    // clear the screen
    clear();

    // the initialization of exception entry
    for(i = 0; i < IDT_END_OF_EXCEPTION; i++){

        // reserve field
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].reserved4 = 0;

        // dpl,size and present bit
        idt[i].dpl = 0;
        idt[i].size = 1;
        idt[i].present = 1;

        // segment selector
        idt[i].seg_selector = KERNEL_CS;
    }

    // the initialization of interrupt and other entry
    for(i = IDT_END_OF_EXCEPTION; i < NUM_VEC; i++){

        // reserve field
        idt[i].reserved0 = 0;
        idt[i].reserved1 = 1;
        idt[i].reserved2 = 1;
        idt[i].reserved3 = 1;
        idt[i].reserved4 = 0;

        // dpl,size and present bit
        idt[i].dpl = 0;
        idt[i].size = 1;
        idt[i].present = 0;

        // segment selector
        idt[i].seg_selector = KERNEL_CS;
    }

    // set all the interrupt handler for exceptions
    SET_IDT_ENTRY(idt[0],exception_handler_0);
    SET_IDT_ENTRY(idt[1],exception_handler_1);
    SET_IDT_ENTRY(idt[2],exception_handler_2);
    SET_IDT_ENTRY(idt[3],exception_handler_3);
    SET_IDT_ENTRY(idt[4],exception_handler_4);
    SET_IDT_ENTRY(idt[5],exception_handler_5);
    SET_IDT_ENTRY(idt[6],exception_handler_6);
    SET_IDT_ENTRY(idt[7],exception_handler_7);
    SET_IDT_ENTRY(idt[8],exception_handler_8);
    SET_IDT_ENTRY(idt[9],exception_handler_9);
    SET_IDT_ENTRY(idt[10],exception_handler_10);
    SET_IDT_ENTRY(idt[11],exception_handler_11);
    SET_IDT_ENTRY(idt[12],exception_handler_12);
    SET_IDT_ENTRY(idt[13],exception_handler_13);
    SET_IDT_ENTRY(idt[14],exception_handler_14);
    SET_IDT_ENTRY(idt[15],exception_handler_15);
    SET_IDT_ENTRY(idt[16],exception_handler_16);
    SET_IDT_ENTRY(idt[17],exception_handler_17);
    SET_IDT_ENTRY(idt[18],exception_handler_18);
    SET_IDT_ENTRY(idt[19],exception_handler_19);

    // Set keyboard handler (defined in boot.S)
    SET_IDT_ENTRY(idt[IDT_ENTRY_KEYBOARD], interrupt_entry_1);
    idt[IDT_ENTRY_KEYBOARD].present = 1;

    // Set RTC handler (defined in boot.S)
    SET_IDT_ENTRY(idt[IDT_ENTRY_RTC], interrupt_entry_8);
    idt[IDT_ENTRY_RTC].present = 1;

    // set the system call entry
    SET_IDT_ENTRY(idt[IDT_SYSTEM_CALL],exception_handler_128);
    idt[IDT_SYSTEM_CALL].dpl = 3;

    LOAD_IDTR(idt_desc_ptr);

}
