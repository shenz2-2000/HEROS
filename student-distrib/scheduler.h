#ifndef SCHEDULE_H
#define SCHEDULE_H

#include "types.h"
#include "process.h"
#include "i8259.h"


#define DUMMY_TASK_NAME "idle_init_name"
#define INIT_PIT_FREQ   300
#define MEM_FENCE       100
#define TIME_DECREASE_PER_INTERRUPT 10
#define TIME_INIT 40
#define TIME_INIT_FOR_SHELL 20
#define N_PCB_OFFSET 10




//
void init_scheduler();
void init_process_time(pcb_t* cur_process);

//void insert_to_list_start(task_node* cur_node);
void append_to_list_end(task_node* cur_node);
void delete_from_list(task_node* cur_node);
void reposition_to_end(task_node* cur_node);
void reschedule();
int32_t add_task_to_list(pcb_t* task);
int32_t delete_task_from_list(pcb_t* task);







#endif
