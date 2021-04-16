//
// Created by BOURNE on 2021/4/16.
//

#ifndef FAKE_SIGNAL_SYS_CALL_H
#define FAKE_SIGNAL_SYS_CALL_H

#endif //FAKE_SIGNAL_SYS_CALL_H

#include "lib.h"
#include "link.h"

#define SIGNAL_NUM 5
#define MASK_ALL 0xFFFFFFFF

typedef int32_t (*signal_handler)(void);
typedef struct signal_t {
    uint32_t signal_pending;
    uint32_t signal_masked;
    uint32_t previous_masked;
    uint32_t alarm_time; // Used for alarm
    signal_handler sig[SIGNAL_NUM]; // Store the signal handle
} signal_struct_t;

// struct for hardware context
typedef struct struct_hw_context {
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
}hw_context;


signal_handler default_handler[SIGNAL_NUM];

// Two System Calls
int32_t sys_set_handler(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn(void); 

// Signal Functions

extern int32_t signal_send(int32_t signum);
extern int32_t signal_mask(int32_t signum);
extern int32_t signal_unmask(int32_t signum);
extern void signal_init(); // Initilize signal system and should be run on boot
extern  int32_t task_signal_init(signal_t* signal_array); // init signal content
