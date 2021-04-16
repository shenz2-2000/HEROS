//
// Created by BOURNE on 2021/4/16.
//

#ifndef FAKE_SIGNAL_SYS_CALL_H
#define FAKE_SIGNAL_SYS_CALL_H

#endif //FAKE_SIGNAL_SYS_CALL_H

#define SIGNAL_NUM 5
typedef int32_t (*signal_handler)(void);
typedef struct signal_t {
    uint32_t signal_pending;
    uint32_t signal_masked;
    uint32_t previous_masked;
    uint32_t alarm_time; // Used for alarm
    signal_handler sig[SIGNAL_NUM]; // Store the signal handle
} signal_struct_t;

// Two System Calls
int32_t sys_set_handler(int32_t signum, void* handler_address);
extern int32_t sys_sigreturn(void); 

// Signal Functions

int32_t signal_send(int32_t signal); 
int32_t signal_block(int32_t signal); 
int32_t signal_unblock(int32_t signal);  
void signal_init(); // Initilize signal system
int32_t task_signal_init(signal_t* signal_array);  // init signal content
