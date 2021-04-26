#include "scheduler.h"
#include "lib.h"
#include "idt.h"
#include "process.h"


static void setup_pit(uint16_t hz);

static int idt_test_cnt = 0;

task_node task_list_head;
task_node all_nodes[N_PCB_LIMIT + N_PCB_OFFSET];

/*
 * init_scheduler
 * This function will initialize task list and task node array
 * Input: None
 * Output: None.
 * Side effect: None.
 */
void init_scheduler() {
    int i;
    setup_pit(INIT_PIT_FREQ);
    task_list_head.next = &task_list_head;
    task_list_head.prev = &task_list_head;
    task_list_head.cur_task = NULL;

    for(i = 0; i < (N_PCB_LIMIT+N_PCB_OFFSET); i++){
        task_node dummy;
        dummy.next = NULL;
        dummy.prev = NULL;
        dummy.valid = 0;
        dummy.cur_task = NULL;
        all_nodes[i] = dummy;
    }

}


// TODO: ensure it works here
/*
 * switch_context
 * This function will switch between processes
 * Input: old: pointer to old esp
 *        new: pointer to new esp
 * Output: None.
 * Side effect: change esp value
 */
#define switch_context(old, new)                                                          \
    asm volatile ("                                                                                         \
    pushl %%ebp                                                                                           \n\
    pushfl                                                                                                \n\
    pushl $0f                                                                                             \n\
    movl %%esp, %0                                                                                        \n\
    movl %1, %%esp                                                                                        \n\
    ret                                                                                                   \n\
0:  popfl                                                                                                 \n\
    popl %%ebp"                                                                                             \
    : "=m" (old)                                                                                            \
    : "r" (new)                                                                                             \
    : "cc", "memory"                                                                                        \
)



// ---------------------------------------------Some tool function for linked list------------------------------------------

/*
 * insert_to_list_start
 * This function will add node to the start
 * Input: cur_node - the node you want to add
 * Output: None.
 * Side effect: change the task list
 */
void insert_to_list_start(task_node* cur_node) {

    // original head
    task_node* temp = task_list_head.next;

    // reinsert
    temp->prev = cur_node;
    cur_node->next = temp;
    cur_node->prev = &task_list_head;
    task_list_head.next = cur_node;
}

/*
 * append_to_list_end
 * This function will append a new node to the end
 * Input: cur_node - the node you want to append
 * Output: None.
 * Side effect: change task list
 */
void append_to_list_end(task_node* cur_node) {

    // original last
    task_node* temp = task_list_head.prev;

    // append
    temp->next = cur_node;
    cur_node->next = &task_list_head;
    cur_node->prev = temp;
    task_list_head.prev = cur_node;
}

/*
 * delete_from_list
 * This function will delete a node from linked list
 * Input: cur_node - the node to be deleted
 * Output: None.
 * Side effect: change the task list
 */
void delete_from_list(task_node* cur_node) {

    // original prev
    task_node* temp_prev = cur_node->prev;
    task_node* temp_next = cur_node->next;

    // delete
    temp_prev->next = temp_next;
    temp_next->prev = temp_prev;

    cur_node->prev = NULL;
    cur_node->next = NULL;

}

/*
 * reposition_to_end
 * This function will reposition a node to end of list
 * Input: cur_node - the node to be repositioned
 * Output: None.
 * Side effect: change task list
 */
void reposition_to_end(task_node* cur_node) {

    delete_from_list(cur_node);
    append_to_list_end(cur_node);

}

/*
 * init_process_time
 * This function will refill the time for a process
 * Input: cur_process - the process to be manipulated
 * Output: None.
 * Side effect: None.
 */
void init_process_time(pcb_t* cur_process){
    cur_process->time = TIME_INIT;
}

/*
 * add_task_to_list
 * add a task to the list and initialize it
 * Input: task - task structure to be added
 * Output: 0 - success
 *        -1 - fail
 * Side effect: change the task list
 */
int32_t add_task_to_list(pcb_t* task){
    int i;
    for(i = 0; i < N_PCB_LIMIT+N_PCB_OFFSET; i++){
        if(all_nodes[i].valid == 0){
            // initialize the node
            all_nodes[i].valid = 1;
            task->node = &all_nodes[i];
            all_nodes[i].cur_task = task;
            append_to_list_end(&all_nodes[i]);
            return 0;

        }
    }

    return -1;
}

/*
 * delete_task_from_list
 * delete a task from task list
 * Input: task - task structure to be deleted
 * Output: 0 - success
 *        -1 - fail
 * Side effect: change the task list
 */
int32_t delete_task_from_list(pcb_t* task){
    int i;
    for(i = 0; i < N_PCB_LIMIT+N_PCB_OFFSET; i++){
        if(all_nodes[i].valid == 1 && all_nodes[i].cur_task == task){
            delete_from_list(&all_nodes[i]);
            all_nodes[i].valid = 0;
            all_nodes[i].cur_task = NULL;
            task->node = NULL;
            return 0;
        }
    }

    return -1;
}



// -------------------------------------------------------------------------------------------------------------------------

/*
 * reschedule
 * reschedule to a new task (or do nothing)
 * Input: None
 * Output: None
 * Side effect: switch stack and modify task list
 */
void reschedule(){

    // next_task to run
    pcb_t* next_task = task_list_head.next->cur_task;

    // sanity check
    if(next_task == NULL){
        printf("WARNING: fail to add task into list!!\n");
        return;
    }

    if(task_list_head.next == &task_list_head){
        printf("WARNING:there should be at least 1 process in list");
    }

    // TODO: fill the idle task name macro
    if( next_task->idle_task ){
        // The first task is idle task, put it to the end of the list
        reposition_to_end(task_list_head.next);
        next_task = task_list_head.next->cur_task;
    }

    if(get_cur_process() == task_list_head.next->cur_task){
        // do nothing, do not need to switch
        return;
    }

    // TODO: not finished yet, page, vidmap, terminal switch

    tss.esp0 = next_task->k_esp;

    // switch stack to the next task
    // TODO: ensure it works here
    switch_context(get_cur_process()->k_esp,next_task->k_esp);

}


/**
 * Interrupt handler of PIT
 * @input hw - hardware context on stack
 * @output None
 * @usage in boot.S
 * @sideeffect modify list and switch stack
 */
ASMLINKAGE void pit_interrupt_handler(hw_context hw) {

    uint32_t eflags;
    pcb_t* cur_task = get_cur_process();

    // check if there is a task
    if( task_list_head.next == &task_list_head ) {
        send_eoi(hw.irq);
        return;
    }

    // critical section
    cli_and_save(eflags);

    // TODO: signal alarm

    // check overflow
    if( ( cur_task->k_esp < ( (uint32_t)cur_task + MEM_FENCE + sizeof(pcb_t) ) ) || ( cur_task->k_esp > TASK_KSTK_SIZE_IN_B + (uint32_t)cur_task)){
        printf("Kernel stack overflow is happening!!\n");
        return;
    }

    // ------------------------------------------------reschedule starts-----------------------------------------------

    // if current running is an idle task
    if( cur_task->idle_task ){
        // reposition cur_task to the end of the linked list
        reposition_to_end(cur_task->node);
        send_eoi(hw.irq);
        reschedule();
    }
    // not idle task
    else{
        cur_task->time -= TIME_DECREASE_PER_INTERRUPT;

        // time is used up
        if(cur_task->time <= 0){
            init_process_time(cur_task);
            reposition_to_end(cur_task->node);
            send_eoi(hw.irq);
            reschedule();
        }
        else{
            send_eoi(hw.irq);
        }
    }

    restore_signal(eflags);

}


/**
 * set up PIT frequency
 * @param hz   desired PIT frequency
 * Reference: http://www.osdever.net/bkerndev/Docs/pit.htm
 */
static void setup_pit(uint16_t hz) {
    uint16_t divisor = 1193180 / hz;    /* Calculate our divisor */
    outb(0x36, 0x34);  // set command byte 0x36
    outb(divisor & 0xFF, 0x40);   // Set low byte of divisor
    outb(divisor >> 8, 0x40);     // Set high byte of divisor
}
