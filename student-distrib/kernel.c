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
// Maximum Size of keyboard buffer should be 128
#define KEYBOARD_BUF_SIZE 128
static int keyboard_buf[KEYBOARD_BUF_SIZE];
static int flag[128]; // There're at most 128 characters on the keyboard
#define RTC_PORT_0    0x70
#define RTC_PORT_1    0x71
#define KEY_BOARD_PRESSED 0x60
#define RELEASE_DIFF      0x80
#define CTRL_PRESSED      0x1D
#define CAPLCL_PRESSED      0x3A
#define LEFT_SHIFT_PRESSED  0x2A
#define RIGHT_SHIFT_PRESSED 0x36
#define L_PRESSED           0x27
#define RTC_LIMIT         10000
#define IDT_ENTRY_KEYBOARD 0x21
#define IDT_ENTRY_RTC 0x28
#define IDT_SYSTEM_CALL 0x80
// Using scan code set 1 for we use "US QWERTY" keyboard
// The table transform scan code to the echo character
static int CUR_CAP = 0; // Keep the current state of caplock
static const char scan_code_table[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,      /* 0x00 - 0x0E */
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',      /* 0x0F - 0x1C */
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',           /* 0x1D - 0x29 */
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0,          /* 0x2A - 0x37 */
        0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x38 - 0x46 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0             /* 0x54 - 0x80 */
};
// The shift_scan_code_table should store the keycode after transform
static const char shift_scan_code_table[128] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',      /* 0x00 - 0x0E */
        0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',      /* 0x0F - 0x1C */
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',           /* 0x1D - 0x29 */
        0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0,          /* 0x2A - 0x37 */
        0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x38 - 0x46 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0             /* 0x54 - 0x80 */
};
// The rtc counter used to count the number of interrupt
static int rtc_counter = 0;
/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags, bit)   ((flags) & (1 << (bit)))


/*
 * naive_exception_handler
 * This function is the handler of an exception
 * Input: num -- exception vector
 * Output: None.
 * Side effect: Print message on screen
 */
void naive_exception_handler(uint32_t num){
    // check whether input is valid
    cli();
    if(num >= 20){
        printf("------------------------------------------------\n");
        printf("WARNING! The Input of Exception is invalid!!!!\n");
        printf("------------------------------------------------\n");
        // shut the screen into blue
        set_blue_screen();
    }
    else {
        printf("------------------------------------------------\n");
        printf("WARNING! EXCEPTION %u HAPPENS!\n", num);
        printf("------------------------------------------------\n");
        // shut the screen into blue
        set_blue_screen();
    }
    while(1){}
    sti();
}

/*
 * naive_system_call_handler
 * This function is the handler of a system call
 * Input: num -- system call vector, which will always be 0x80
 * Output: None.
 * Side effect: Print message on screen
 */
void naive_system_call_handler(uint32_t num){
    if(num != IDT_SYSTEM_CALL){
        printf("------------------------------------------------\n");
        printf("SYSTEM CALL INVALID\n");
        printf("------------------------------------------------\n");
    }
    else {
        printf("------------------------------------------------\n");
        printf("SYSTEM CALL HAPPENS\n");
        printf("------------------------------------------------\n");
    }
}


/*
 * entry
 * This function will be called after boot.s
 * Input: None
 * Output: None.
 * Side effect: Initialize the system and do the tests
 */
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

    /* init paging */
    fill_page();
    init_page_register();

    /* Init the PIC */
    i8259_init();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    enable_irq(1);   // Keyboard is IRQ1

    /*Init RTC*/
    rtc_init();
    enable_irq(8); // RTC is IRQ8
    rtc_restart_interrupt();

    /*Init IDT*/
    init_IDT();

    /* Enable interrupts */

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
    int capital;
    // Get input from keyboard and check whether the scan code can be output
    if (input > KEY_BOARD_PRESSED) {
        flag[input - RELEASE_DIFF] = 0; 
        if (input - RELEASE_DIFF == CAPLCL_PRESSED) CUR_CAP^=1;
    else {
        flag[input] = 1;
        capital = CUR_CAP ^ (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
        if (flag[CTRL_PRESSED]&&flag[L_PRESSED]) {
            clear();
            reset_screen();
        } else if (scan_code_table[input]) putc(capital?shift_scan_code_table[input]:scan_code_table[input]);
    }
        
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
extern int get_rtc_counter(){
    return rtc_counter;
}
