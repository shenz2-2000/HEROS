//
// Created by BOURNE on 2021/3/23.
//
#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "file_sys.h"
#include "link.h"
#include "process.h"
#include "sys_call.h"
#include "page_lib.h"
#include "idt.h"
/* Declaration of constant in interrupt */

#define IDT_ENTRY_PIT      0x20
#define IDT_ENTRY_KEYBOARD 0x21
#define IDT_ENTRY_SB16     0x25
#define IDT_ENTRY_RTC 0x28
#define IDT_ENTRY_MOUSE 0x2C
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

    SET_IDT_ENTRY(idt[IDT_ENTRY_PIT], interrupt_entry_0);
    idt[IDT_ENTRY_PIT].present = 1;

    // Set keyboard handler (defined in boot.S)
    SET_IDT_ENTRY(idt[IDT_ENTRY_KEYBOARD], interrupt_entry_1);
    idt[IDT_ENTRY_KEYBOARD].present = 1;

    // Set sound card interrupt handler (defined in boot.S)
    SET_IDT_ENTRY(idt[IDT_ENTRY_SB16], interrupt_entry_5);
    idt[IDT_ENTRY_SB16].present = 1;

    // Set RTC handler (defined in boot.S)
    SET_IDT_ENTRY(idt[IDT_ENTRY_RTC], interrupt_entry_8);
    idt[IDT_ENTRY_RTC].present = 1;

    // Set mouse handler
    SET_IDT_ENTRY(idt[IDT_ENTRY_MOUSE], interrupt_entry_12);
    idt[IDT_ENTRY_MOUSE].present = 1;

    // set the system call entry
    SET_IDT_ENTRY(idt[IDT_SYSTEM_CALL],sys_call_linkage);
    idt[IDT_SYSTEM_CALL].dpl = 3;
    idt[IDT_SYSTEM_CALL].present = 1;

    LOAD_IDTR(idt_desc_ptr);

}

// Ten system calls


/* int32_t open_sys_call(const uint8_t* filename)
 * Inputs: filename : the file we want to open
 * Return Value: ret: whether the open is successful
 */

ASMLINKAGE int32_t open_sys_call(const uint8_t* filename){
    return sys_open(filename);
}

/* int32_t read_sys_call(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd : file descriptor
 *         buf: container provided by caller
 *         nbytes: number of bytes we want to read
 * Return Value: ret: whether the read is successful
 */

ASMLINKAGE int32_t read_sys_call(int32_t fd, void* buf, int32_t nbytes){
    return sys_read(fd,buf,nbytes);
}

/* int32_t close_sys_call(int32_t fd)
 * Inputs: fd : the file we want to close
 * Return Value: ret: whether the close is successful
 */

ASMLINKAGE int32_t close_sys_call(int32_t fd){
    return sys_close(fd);
}

/* int32_t write_sys_call(int32_t fd, void* buf, int32_t nbytes)
 * Inputs: fd : file descriptor
 *         buf: container provided by caller
 *         nbytes: number of bytes we want to write
 * Return Value: ret: whether the write is successful
 */
ASMLINKAGE int32_t write_sys_call(int32_t fd, const void* buf, int32_t nbytes){
    uint32_t flags;
    int32_t ret;
    cli_and_save(flags);
    ret = sys_write(fd,buf,nbytes);
    restore_flags(flags);
    return ret;
}

ASMLINKAGE int32_t ioctl_sys_call(int32_t fd, int32_t cmd) {
    return sys_ioctl(fd, cmd);
}

// extern int sys_execute(uint8_t *command);

ASMLINKAGE int dummy_sys_call(){
    return invalid_sys_call();
}

ASMLINKAGE int32_t halt_sys_call(int8_t status){

    int32_t eflags;
    int32_t return_val;

    cli_and_save(eflags);
    return_val = system_halt((int32_t)status);
    restore_flags(eflags);

    return return_val;
}

ASMLINKAGE int32_t execute_sys_call( uint8_t *command){
    uint32_t flags;
    int32_t ret;

    cli_and_save(flags);
    ret = sys_execute(command, 1, 0, NULL);
    restore_flags(flags);

    return ret;
}

ASMLINKAGE int32_t get_args_sys_call(uint8_t *buf, int32_t nbytes){
    return sys_get_args(buf,nbytes);
}

ASMLINKAGE int32_t vidmap_sys_call(uint8_t ** screen_start){
    uint32_t flags;
    int32_t ret;
    cli_and_save(flags);
    ret = sys_vidmap(screen_start);
    restore_flags(flags);
    return ret;
}

ASMLINKAGE int32_t set_handler_sys_call(int32_t signum, void* handler_address){
    return sys_set_handler(signum, handler_address);
}

ASMLINKAGE int32_t sig_return_sys_call(){
    return sys_sig_return();
}

ASMLINKAGE int32_t play_sound_sys_call(uint32_t nFrequence) {
    return sys_play_sound(nFrequence);
}

ASMLINKAGE int32_t nosound_sys_call() {
    return sys_nosound();
}
