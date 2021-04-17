//
// Created by BOURNE on 2021/4/16.
//

#ifndef FAKE_SIGNAL_SYS_CALL_H
#define FAKE_SIGNAL_SYS_CALL_H


#define SIGNAL_NUM 5
#define MASK_ALL 0xFFFFFFFF


#ifndef ASM_SIGNAL

#include "idt.h"
#include "types.h"
// #include "types.h"

typedef int32_t (*signal_handler)(void);
typedef struct signal_struct_t {
    uint32_t signal_pending;
    uint32_t signal_masked;
    uint32_t previous_masked;
    uint32_t alarm_time; // Used for alarm
    signal_handler sig[SIGNAL_NUM]; // Store the signal handle
} signal_struct_t;


signal_handler default_handler[SIGNAL_NUM];

// Two System Calls
int32_t sys_set_handler(int32_t signum, void* handler_address);
int32_t sys_sigreturn(void);

void restore_signal(void);

// Signal Functions
int32_t signal_send(int32_t signum);
int32_t signal_mask(int32_t signum);
int32_t signal_unmask(int32_t signum);
void signal_init(); // Initilize signal system and should be run on boot
int32_t task_signal_init(signal_struct_t* signal_array); // init signal content

#endif
#endif //FAKE_SIGNAL_SYS_CALL_H
