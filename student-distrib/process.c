#include "process.h"
#include "file_sys.h"
#include "page_lib.h"
#include "lib.h"

pcb_t* pcb_ptrs[N_PCB_LIMIT];
int32_t n_present_pcb;

/**
 * process_init
 * Description: Initialize the process array system
 * Input: None
 * Output: 0 if success.
 * Side effect: all processes are set not present
 */
void process_init() {
    int i;
    n_present_pcb = 0;
    for (i = 0; i < N_PCB_LIMIT; i++) {
        pcb_ptrs[i] = (pcb_t*) (TASK_KSTK_BOTTOM - (i+1) * TASK_KSTK_SIZE_IN_B);
        pcb_ptrs[i]->present = 0;   // init as not present
    }
}

/**
 * get_cur_process
 * Description: obtain the current running process
 * Input: None
 * Output: the address of the current pcb
 * Side effect: None
 */
pcb_t* get_cur_process() {
    pcb_t *ret;
    asm volatile (
        "movl %%esp, %0" \
        : "=r" (ret) \
        : \
        : "cc", "memory" \
    );
    ret = (pcb_t*) (((int32_t) ret) & TASK_KSTK_PCB_ADDR_MASK);
    return ret;
}

/**
 * create_process
 * Description: create a new pcb and return the pcb pointer
 * Input: None
 * Output: the pointer to the new pcb
 * Side effect: everything in the pcb is not initialized
 */
pcb_t* create_process() {
    int i;

    if (n_present_pcb >= N_PCB_LIMIT) {
        printf("ERROR in create_process: number of process limit reached");
        return NULL;
    }
    n_present_pcb += 1;

    for (i = 0; i < N_PCB_LIMIT; i++) {
        if (!(pcb_ptrs[i]->present)) {
            pcb_ptrs[i]->present = 1;
            return pcb_ptrs[i];
        }
    }

    printf("ERROR in create_process: somethings wrong");
    return NULL;
}

/**
 * delete_process
 * Description: delete the process
 * Input: the pcb of target process
 * Output: the parent of the deleted process
 * Side effect: the process is deleted
 */
pcb_t* delete_process(pcb_t* pcb) {
    pcb->present = 0;
    n_present_pcb -= 1;
    return pcb->parent;
}

/**
 * system_halt
 * Description: Implementation of halt()
 * Input: status - exit status info
 * Output: 0 if success.
 * Return: should never return
 * Side effect: terminate the process and return value to its parent
 */
int32_t system_halt(int32_t status) {
    // condition check
    if (n_present_pcb == 0) {   // not in a process
        return -1;
    }
    if (get_cur_process()->parent == NULL) {
        //TODO: if the last program has been halt, don't know if we need to consider this
    }

    close_all_files(&get_cur_process()->file_arr);  // close FDs
    pcb_t *parent = delete_process(get_cur_process());  // clear the pcb
    restore_paging(get_cur_process()->pid, parent->pid);  // restore parent paging
    tss.esp0 = parent->k_esp;  // set tss to parent's kernel stack to make sure system calls use correct stack

    asm volatile ("                                                                    \
        movl %0, %%esp  /* load old ESP */                                           \n\
        ret  /* now it's equivalent to jump execute return */"                          \
        :                                                                              \
        : "r" (parent->k_esp)   , "a" (status)                                         \
        : "cc", "memory"                                                               \
    );

    return -1;
}
