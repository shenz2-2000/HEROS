#ifndef _PROCESS_H
#define _PROCESS_H

#include "file_sys.h"
#include "x86_desc.h"
#include "signal_sys_call.h"
#define N_PCB_LIMIT 3

#define TASK_KSTK_SIZE_IN_B 8192
#define TASK_KSTK_BOTTOM 0x800000    
#define TASK_KSTK_PCB_ADDR_MASK 0xFFFFE000
#define US_STARTING  (0x8400000 - 1)

// the definition of linked list's node
typedef struct task_node{
    struct task_node* prev;
    struct task_node* next;
    struct pcb_t * cur_task;
    int valid;
}task_node;


typedef struct pcb_t pcb_t;
struct pcb_t {
    uint8_t present;
    uint8_t vidmap_enable;
    pcb_t* parent;
    uint8_t* name;
    uint8_t* args;
    uint32_t k_esp;
    file_arr_t file_arr;
    uint8_t pid;
    int init_task,kernel_task,idle_task;
    signal_struct_t signals;
    uint32_t time;
    task_node* node;
    terminal_struct_t* terminal;
};


typedef union task_kstack_t {
    pcb_t pcb;
    uint8_t stk[TASK_KSTK_SIZE_IN_B];
} task_kstack_t;

void process_init();
pcb_t* get_cur_process();
pcb_t* focus_task();
pcb_t* create_process();
pcb_t* delete_process(pcb_t* pcb);
int32_t get_n_present_pcb();
void change_focus_task(int32_t terminal_id);
#endif
