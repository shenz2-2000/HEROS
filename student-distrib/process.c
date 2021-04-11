#include "process.h"
#include "file_sys.h"
#include "page_lib.h"
#include "lib.h"
#include "tests.h"

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
 * create_process
 * Description: create a new pcb and return the pcb pointer
 * Input: None
 * Output: the pointer to the new pcb
 * Side effect: everything in the pcb is not initialized
 */
pcb_t* create_process() {
    int i;

    if (n_present_pcb >= N_PCB_LIMIT) {
        printf("ERROR in create_process: number of process limit reached\n");
        return NULL;
    }
    n_present_pcb += 1;

    for (i = 0; i < N_PCB_LIMIT; i++) {
        if (!(pcb_ptrs[i]->present)) {
            pcb_ptrs[i]->present = 1;
            return pcb_ptrs[i];
        }
    }

    printf("ERROR in create_process: somethings wrong\n");
    return NULL;
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
 * parse_args
 * Description: parse the input character to 
 * obtain separate command and args
 * Input: command -- string to be splited
 * Output: args -- pointer to argument string
 *         command -- name of execuatable command 
 * Side effect: None
 */
int parse_args(uint8_t *command, uint8_t **args){
    // bad input
    if (command == NULL) {
        printf("ERROR in parse_args: command NULL pointer\n");
        return -1;
    }
    // Possible to have no arguments
    *args = NULL;
    while(*command !='\0') {
        if (*command == ' ') {
            *command = '\0'; // End here
            *args = command + 1;
            return 0;
        }
        ++command;
    }
    return 0;
}

/**
 * sys_execute
 * Description: execute the command
 * Input: command -- string to be executed
 * Output: None
 * Side effect: Run the new program with the command
 */

int sys_execute(uint8_t *command) {
    pcb_t *process;
    int32_t eip;
    uint32_t length;
    int pid_ret;
    int ret;

    // bad input
    if (command == NULL) {
        printf("ERROR in sys_execute: command NULL pointer\n");
        return -1;
    }

    process = create_process();
    if (process==NULL) return -1; // Raise Error
    // Information Setting
    process->parent = n_present_pcb==1?NULL:get_cur_process();
    process->k_esp = (uint32_t)process+TASK_KSTK_SIZE_IN_B-1;
    parse_args(command, &(process->args));
    process->name = command;
    // move name and argument to new program's PCB
    length = strlen((int8_t *)process -> name);
    process->k_esp-=length;
    process->name=(uint8_t *)strcpy((int8_t *)process->k_esp, (int8_t *) process->name);
    if (process->args!=NULL) {
        length = strlen((int8_t *)process -> args);
        process->k_esp-=length;
        process->args=(uint8_t *)strcpy((int8_t *)process->k_esp, (int8_t *) process->args);
    } 
    pid_ret = set_page_for_task(command, (uint32_t *)&eip);
    if (pid_ret < 0) {
        delete_process(process);
        return -1;
    }
    process->pid = pid_ret;
    init_file_arr(&(process->file_arr));
    // Set up tss to make sure system call don't go wrong
    tss.ss0 = KERNEL_DS;
    tss.esp0 = process->k_esp;

    if (process->parent==NULL) asm volatile ("                                                                 \
    pushfl                                                                          \n\
    pushl %%ebp                                                                     \n\
    pushl $1f       /* return to label 1 for continue execute */                    \n\
    movl %%esp, %0                                                                  \n\
    /*IRET Preparation*/                                                            \n\
    pushl $0x002B   /* USER_DS */                                                   \n\
    pushl %3                                                                        \n\
    pushf                                                                           \n\
    pushl $0x0023   /* USER_CS  */                                                  \n\
    pushl %2                                                                        \n\
    iret                                                                            \n\
1:  popl %%ebp                                                                      \n\
    movl %%eax, %1                                                                  \n\
    popfl      "                                                                       \
    : "=m" (length), /* must write to memory*/      \
      "=m" (ret)                                                                      \
    : "rm" (eip), "rm" (US_STARTING)                                                  \
    : "cc", "memory"                                                                  \
    ); else asm volatile ("                                                                 \
    pushfl                                                                          \n\
    pushl %%ebp                                                                     \n\
    pushl $1f       /* return to label 1 for continue execute */                    \n\
    movl %%esp, %0                                                                  \n\
    /*IRET Preparation*/                                                            \n\
    pushl $0x002B   /* USER_DS */                                                   \n\
    pushl %3                                                                        \n\
    pushf                                                                           \n\
    pushl $0x0023   /* USER_CS  */                                                  \n\
    pushl %2                                                                        \n\
    iret                                                                            \n\
1:  popl %%ebp                                                                      \n\
    movl %%eax, %1                                                                  \n\
    popfl      "                                                                       \
    : "=m" (get_cur_process()->k_esp), /* must write to memory*/      \
      "=m" (ret)                                                                      \
    : "rm" (eip), "rm" (US_STARTING)                                                  \
    : "cc", "memory"                                                                  \
    );
    return ret;
}

/**
 * delete_process
 * Description: delete the process
 * Input: the pcb of target process
 * Output: the parent of the deleted process
 * Side effect: the process is deleted
 */
pcb_t* delete_process(pcb_t* pcb) {
    if (pcb == NULL) {
        printf("ERROR in delete_process: pcb NULL pointer\n");
        return NULL;
    }
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
        // So the main program of base shell return on exit, so we need to execute shell again
        printf("Warning in system_halt(): halting the base shell...\n");
        close_all_files(&get_cur_process()->file_arr);  // close FDs
        delete_process(get_cur_process());
        restore_paging(get_cur_process()->pid, get_cur_process()->pid); // restore the current paging
        sys_execute((uint8_t *) "shell");
        return -1;
    }

    // CHECK ALL FILES ARE CLEANED UP
    check_all_files_closed();

    close_all_files(&get_cur_process()->file_arr);  // close FDs
    pcb_t *parent = delete_process(get_cur_process());  // clear the pcb
    restore_paging(get_cur_process()->pid, parent->pid);  // restore parent paging
    tss.esp0 = parent->k_esp;  // set tss to parent's kernel stack to make sure system calls use correct stack

    // load esp and return
    asm volatile ("                                                                    \
        movl %0, %%esp  /* load old ESP */                                           \n\
        ret  /* now it's equivalent to jump execute return */"                          \
        :                                                                              \
        : "r" (parent->k_esp)   , "a" (status)                                         \
        : "cc", "memory"                                                               \
    );

    printf("In system_halt: this function should never return.\n");
    return -1;
}
