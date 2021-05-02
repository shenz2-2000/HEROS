//
// Created by 10656 on 2021/4/17.
//

#ifndef STUDENT_DISTRIB_IDT_H
#define STUDENT_DISTRIB_IDT_H

#ifndef ASM
#include "types.h"
#include "link.h"

//// struct for hardware context
typedef struct hw_context {
    int32_t ebx;
    int32_t ecx;
    int32_t edx;
    int32_t esi;
    int32_t edi;
    int32_t ebp;
    int32_t eax;
    int32_t ds;
    int32_t es;
    int32_t fs;
    int32_t irq;
    int32_t error;
    int32_t eip;
    int32_t cs;
    int32_t eflags;
    int32_t esp;
    int32_t ss;
} hw_context;

// init IDT
extern void init_IDT();

// two naive handlers
extern void naive_exception_handler(hw_context hw);
extern void naive_system_call_handler(uint32_t num);

// declaration of the filling function
extern void fill_page();
extern void init_page_register();

// declaration of the exception_handlers
extern void exception_handler_0();
extern void exception_handler_1();
extern void exception_handler_2();
extern void exception_handler_3();
extern void exception_handler_4();
extern void exception_handler_5();
extern void exception_handler_6();
extern void exception_handler_7();
extern void exception_handler_8();
extern void exception_handler_9();
extern void exception_handler_10();
extern void exception_handler_11();
extern void exception_handler_12();
extern void exception_handler_13();
extern void exception_handler_14();
extern void exception_handler_15();
extern void exception_handler_16();
extern void exception_handler_17();
extern void exception_handler_18();
extern void exception_handler_19();

// naive system call handler
extern void exception_handler_128();

// declration of system calls
//extern int32_t halt_sys_call(uint8_t status);
//extern int32_t execute_sys_call(const uint8_t* command);
//extern int32_t read_sys_call(int32_t fd, void* buf, int32_t nbytes);
//extern int32_t write_sys_call(int32_t fd, const void* buf, int32_t nbytes);
//extern int32_t open_sys_call(const uint8_t* filename);
//extern int32_t close_sys_call(int32_t fd);
//extern int32_t get_args_sys_call(uint8_t *buf, int32_t nbytes);
//extern int32_t vidmap_sys_call(uint8_t ** screen_start);
//extern int32_t set_handler_sys_call(int32_t signum, void* handler_address);
//extern int32_t sig_return_sys_call(void);
//
//extern void dummy_sys_call(void);

// system call linkage
extern void sys_call_linkage();

// Interrupt Handler
extern void interrupt_entry_0();    // PIT
extern void interrupt_entry_1();
extern void interrupt_entry_5();
extern void interrupt_entry_8();
extern void interrupt_entry_12();

#endif
#endif //STUDENT_DISTRIB_IDT_H
