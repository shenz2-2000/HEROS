/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"

#define RUN_TESTS

/* Function Declaration */
void idt_init();
void keyboard_interrupt_handler();
void rtc_interrupt_handler();
void rtc_init();
void rtc_restart_interrupt();
/* Declaration of constant in interrupt */
#define KEYBOARD_PORT 0x60
#define RTC_PORT_0    0x70
#define RTC_PORT_1    0x71
#define KEY_BOARD_PRESSED 0x60
#define RTC_LIMIT         300
#define IDT_ENTRY_KEYBOARD 0x21
#define IDT_ENTRY_RTC 0x28
#define IDT_SYSTEM_CALL 0x80
// Using scan code set 1 for we use "US QWERTY" keyboard
// The table transform scan code to the echo character
static const char scan_code_table[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,      /* 0x00 - 0x0E */
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',      /* 0x0F - 0x1C */
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',           /* 0x1D - 0x29 */
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0,          /* 0x2A - 0x37 */
        0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x38 - 0x46 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                                    /* 0x47 - 0x53 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0                                        /* 0x54 - 0x80 */
};
// The rtc counter used to count the number of interrupt
static int rtc_counter = 0;
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))


void naive_exception_handler(uint32_t num){
//    clear();
    printf("------------------------------------------------\n");
    printf("WARNING! EXCEPTION %u HAPPENS!\n",num);
    printf("------------------------------------------------\n");
    while(1){}
}

void naive_system_call_handler(uint32_t num){
    clear();
    printf("------------------------------------------------\n");
    printf("SYSTEM CALL HAPPENS\n");
    printf("------------------------------------------------\n");
}


void init_IDT(){
    clear(); ////
    printf("now in the start of INITIDT\n");
    int i;

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
    printf("The initialization of IDT is finished!!\n");
    // clear();
}



/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void entry(unsigned long magic, unsigned long addr) {

    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
        printf("Invalid magic number: 0x%#x\n", (unsigned)magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf("flags = 0x%#x\n", (unsigned)mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG(mbi->flags, 0))
        printf("mem_lower = %uKB, mem_upper = %uKB\n", (unsigned)mbi->mem_lower, (unsigned)mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG(mbi->flags, 1))
        printf("boot_device = 0x%#x\n", (unsigned)mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG(mbi->flags, 2))
        printf("cmdline = %s\n", (char *)mbi->cmdline);

    if (CHECK_FLAG(mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while (mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for (i = 0; i < 16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
            mod++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG(mbi->flags, 4) && CHECK_FLAG(mbi->flags, 5)) {
        printf("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG(mbi->flags, 5)) {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);
        printf("elf_sec: num = %u, size = 0x%#x, addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned)elf_sec->num, (unsigned)elf_sec->size,
                (unsigned)elf_sec->addr, (unsigned)elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG(mbi->flags, 6)) {
        memory_map_t *mmap;
        printf("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned)mbi->mmap_addr, (unsigned)mbi->mmap_length);
        for (mmap = (memory_map_t *)mbi->mmap_addr;
                (unsigned long)mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *)((unsigned long)mmap + mmap->size + sizeof (mmap->size)))
            printf("    size = 0x%x, base_addr = 0x%#x%#x\n    type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned)mmap->size,
                    (unsigned)mmap->base_addr_high,
                    (unsigned)mmap->base_addr_low,
                    (unsigned)mmap->type,
                    (unsigned)mmap->length_high,
                    (unsigned)mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity = 0x0;
        the_ldt_desc.opsize      = 0x1;
        the_ldt_desc.reserved    = 0x0;
        the_ldt_desc.avail       = 0x0;
        the_ldt_desc.present     = 0x1;
        the_ldt_desc.dpl         = 0x0;
        the_ldt_desc.sys         = 0x0;
        the_ldt_desc.type        = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity   = 0x0;
        the_tss_desc.opsize        = 0x0;
        the_tss_desc.reserved      = 0x0;
        the_tss_desc.avail         = 0x0;
        the_tss_desc.seg_lim_19_16 = TSS_SIZE & 0x000F0000;
        the_tss_desc.present       = 0x1;
        the_tss_desc.dpl           = 0x0;
        the_tss_desc.sys           = 0x0;
        the_tss_desc.type          = 0x9;
        the_tss_desc.seg_lim_15_00 = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /* Init the PIC */
    i8259_init();
    

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    enable_irq(1);   // Keyboard is IRQ1
    /*Init RTC*/
    rtc_init();
    enable_irq(8); // RTC is IRQ8
    rtc_restart_interrupt();

    /* Enable interrupts */
    init_IDT();
    puts("IDT initialized successfully");
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise QEMU will triple fault and simple close
     * without showing you any output */
    /*printf("Enabling Interrupts\n");
    sti();*/

#ifdef RUN_TESTS
    /* Run tests */
    launch_tests();
#endif
    /* Execute the first program ("shell") ... */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile (".1: hlt; jmp .1;");
}

/*
 * keyboard_interrupt
 *   DESCRIPTION: Handle the interrupt from keyboard 
 *   INPUTS: scan code from port 0x60
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: interrupt
 */

void keyboard_interrupt_handler() {
    cli();
    uint8_t input = inb(KEYBOARD_PORT);
    if (input < KEY_BOARD_PRESSED) 
        if (scan_code_table[input]) 
            putc(scan_code_table[input]);
    sti();
}

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
    if (rtc_counter>=RTC_LIMIT){
        printf("RECEIVE %d RTC Interrupts\n", rtc_counter);
        rtc_counter = 0;
    }
    rtc_restart_interrupt();
    sti();
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
extern int get_rtc_counter(){
    return rtc_counter;
}
