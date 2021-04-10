#ifndef _PROCESS_H
#define _PROCESS_H

#include "file_sys.h"

#define N_PCB_LIMIT 2

#define TASK_KSTK_SIZE_IN_B 8192
#define TASK_KSTK_BOTTOM 0x800000    
#define TASK_KSTK_PCB_ADDR_MASK 0xFFFFE000
#define US_STARTING  (0x8400000 - 1)
typedef struct pcb_t {
    uint8_t present;
    pcb_t* parent;
    
    uint8_t* name;
    uint8_t* args;
    uint32_t k_esp;

    file_arr_t file_arr;
} pcb_t;

typedef union task_kstack_t {
    pcb_t pcb;
    uint8_t stk[TASK_KSTK_SIZE_IN_B];
} task_kstack_t;

#endif
