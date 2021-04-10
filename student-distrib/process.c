#include "process.h"
#include "file_sys.h"
pcb_t* pcb_ptrs[N_PCB_LIMIT];
int32_t n_present_pcb;

/**
 * process_init
 * Description: Initialize the process array system
 * Input: None
 * Output: 0 if success.
 * Side effect: Initialize the file system
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
 * Side effect: Initialize the file system
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
int parse_args(char *command, char **args){
    // Possible to have no arguments
    *arg = NULL;
    while(*command !=\'0') {
        if (*command == ' ') {
            *command = '\0'; // End here
            *args = command + 1;
            return 0;
        }
        ++command;
    }
    return 0;
}

int execute_launch(uint32_t prev_kesp, uint32_t esp_nxt, uint32_t eip_nxt) {
    int ret;
    asm volatile ("pushfl                                                           \n\
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
    popfl                                                                             \
    : "=m" (prev_kesp), /* must write to memory*/      \
      "=m" (ret)                                                                      \
    : "rm" (eip_nxt), "rm" (esp_nxt)                                                  \
    : "cc", "memory"                                                                  \
)

    return ret;
}
/**
 * sys_execute
 * Description: execute the command
 * Input: command -- string to be executed
 * Output: None
 * Side effect: Run the new program with the command
 */

int sys_execute(char *command) {
    pcb_t *process;
    int32_t eip;
    int length, ret;
    process = create_process();
    if (process==NULL) return -1; // Raise Error
    // Information Setting
    process->parent = n_present_pcb==1?NULL:get_cur_process();
    process->k_esp = (uint32_t)process+TASK_KSTK_SIZE_IN_B-1;
    parse_args(command, &(process->args));
    process->name = command;
    // move name and argument to new program's PCB
    length = strlen(process -> name);
    process->k_esp-=length;
    process->name=(uint8_t *)strcpy((int8_t *)process->k_esp, (int8_t *) process->name);
    if (process->args!=NULL) {
        length = strlen(process -> args);
        process->k_esp-=length;
        process->args=(uint8_t *)strcpy((int8_t *)process->k_esp, (int8_t *) process->args);
    } 
    process->page_num = set_up_memory_for_task(command, &eip);
    if (process->page_num) {
        delete_process(process);
        return -1;
    }
    init_file_arr(&(process->file_arr));
    // Set up tss to make sure system call don't go wrong
    tss.ss0 = KERNEL_DS;
    tss.esp0 = process->k_esp;
    if (process->parent==NULL) ret = launch_program(NULL, US_STARTING, eip);
    else ret = launch_program(get_cur_process()->k_esp, US_STARTING, eip);
    return ret;
}