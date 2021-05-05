#ifndef _PROCESS_H
#define _PROCESS_H
#include "gui.h"
#include "file_sys.h"
#include "x86_desc.h"
#include "signal_sys_call.h"
#include "terminal.h"
#define N_PCB_LIMIT 10

#define TASK_KSTK_SIZE_IN_B 8192
#define TASK_KSTK_BOTTOM 0x800000    
#define TASK_KSTK_PCB_ADDR_MASK 0xFFFFE000
#define US_STARTING  (0x8400000 - 1)

// vidmap for SVGA
#define VM_BUF_SVGA_ADDR    0xE0000 // the start of 3 buf
#define VM_BUF_SVGA_PD_INDEX    0x00   // addr >> 22
#define VM_BUF_SVGA_PT_INDEX    0xE0

#define SIZE_4K_IN_BYTES    0x1000

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
    pcb_t* parent;
    uint8_t* name;
    uint8_t* args;
    uint32_t k_esp, k_esp_base;
    file_arr_t file_arr;
    uint8_t pid;
    uint8_t init_task,kernel_task,idle_task,own_terminal,wait_for_child;
    uint8_t having_child_running;
    signal_struct_t signals;
    uint32_t time;
    task_node* node;
    terminal_struct_t* terminal;
    int8_t rtc_id;
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
int parse_args(uint8_t *command, uint8_t **args);
int32_t system_halt(int32_t status);
pcb_t *get_showing_task();
int32_t sys_execute(uint8_t *command, int wait_for_child, int separate_terminal, void (*function_address)());
void change_focus_terminal(int32_t terminal_id);
terminal_struct_t* get_running_terminal();
void set_running_terminal(terminal_struct_t* cur);
void init_task_main();
void process_user_vidmap(pcb_t *process);
void update_screen();
extern window_t terminal_window[3];;
extern int need_play;
#endif
