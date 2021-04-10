#ifndef _PROCESS_H
#define _PROCESS_H

#include "file_sys.h"

#define N_PCB_LIMIT 2

#define TASK_KSTK_SIZE_IN_B 8192
#define TASK_KSTK_BOTTOM 0x800000    
#define TASK_KSTK_PCB_ADDR_MASK 0xFFFFE000


typedef struct pcb_t pcb_t;
struct pcb_t {
    uint8_t present;
    pcb_t* parent;
    
    uint8_t* name;
    uint8_t* args;
    uint32_t k_esp;

    file_arr_t file_arr;
};

typedef union task_kstack_t {
    pcb_t pcb;
    uint8_t stk[TASK_KSTK_SIZE_IN_B];
} task_kstack_t;

void process_init();
pcb_t* get_cur_process();
pcb_t* create_process();
pcb_t* delete_process(pcb_t* pcb);


int32_t system_halt(int32_t status);

#endif
