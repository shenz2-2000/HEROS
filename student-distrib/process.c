#include "process.h"
#include "file_sys.h"
#include "page_lib.h"
#include "lib.h"
#include "sys_call.h"
#include "scheduler.h"
#include "mouse_driver.h"
#include "gui.h"
#include "wav_player.h"
#include "mouse_driver.h"

pcb_t* pcb_ptrs[N_PCB_LIMIT];
int32_t n_present_pcb;
pcb_t *cur_task = NULL;
pcb_t *foreground_task[MAX_TERMINAL];
extern terminal_struct_t null_terminal;
terminal_struct_t *running_term = &null_terminal;
terminal_struct_t *showing_term = &null_terminal;
int32_t all_terminal_is_on = 0 ;
static int init = 0;
/**
 * get_running_terminal
 * Description: return the terminal of the task that is running
 * Input: None
 * Output: running_term
 */
terminal_struct_t* get_running_terminal() {
    return running_term;
}
/**
 * set_running_terminal
 * Description: set the terminal to be the input terminal
 * Input: None
 * Output: none
 */
void set_running_terminal (terminal_struct_t* cur) {
    running_term = cur;
}
/**
 * get_showing_task
 * Description: return the current task that is showing
 * Input: None
 * Output: cur_task
 */
pcb_t *get_showing_task() {
    return cur_task;
}

void process_user_vidmap(pcb_t *process) {
    // sanity check
    if (process == NULL) {
        printf("ERROR in process_user_vidmap(): NULL input");
        return;
    }
    set_video_memory_SVGA(process->terminal);
}

/**
 * set_showing_task
 * Description: change the task that we are showing
 * Input: target_task -- the task we want to set
 * Output: None
 */
void set_showing_task(pcb_t* target_task) {
    if (target_task!=NULL && target_task->terminal==NULL) return;
    uint32_t flags;
    cli_and_save(flags);
    //terminal_struct_t* old_term = get_showing_task()==NULL?&null_terminal:get_showing_task()->terminal;
    terminal_struct_t* new_term = target_task==NULL?&null_terminal:target_task->terminal;
    // because it's SVGA version, we don't switch terminal
    // switch_terminal(old_term,new_term);
    showing_term = new_term;    // sync the terminal_showing
    cur_task = target_task;
    process_user_vidmap(get_cur_process());
    restore_flags(flags);
}

/**
 * change_focus_terminal
 * Description: switch terminal
 * Input: terminal_num -- the terminal id want to change to
 * Output: showing_task
 */
void change_focus_terminal(int32_t terminal_num) {
    set_showing_task(foreground_task[terminal_num]);
}

#define execute_user_program_asm(esp_des, esp, eip, ret) asm volatile ("                    \
    pushfl          /* save flags */                                   \n\
    pushl %%ebp     /* save EBP*/                                      \n\
    pushl $0f                                                               \n\
    movl %%esp, %0                                                        \n\
    pushl $0x002B                                                         \n\
    pushl %2                                                            \n\
    pushl $0x206                                                     \n\
    pushl $0x0023                                                    \n\
    pushl %3                                                          \n\
    iret                                                              \n\
0:  popl %%ebp                                                        \n\
    movl %%eax, %1                                                     \n\
    popfl   "                                              \
    : "=m" (esp_des),                                                            \
      "=m" (ret)                                                                      \
    : "rm" (esp), "rm" (eip)                                                  \
    : "cc", "memory"                                                                  \
)

#define execute_program_in_kernel_asm(esp_des, esp, eip, ret) asm volatile (" \
    pushfl                                                  \n\
    pushl %%ebp                                                    \n\
    pushl $0f                                                       \n\
    movl %%esp, %0                                           \n\
    movl %2, %%esp                                                \n\
    pushl %3                                                   \n\
    pushl $0x206                                                   \n\
    popfl                                                                           \n\
    ret                                                        \n\
0:  popl %%ebp                                                  \n\
    movl %%eax, %1                                               \n\
    popfl                           "                                              \
    : "=m" (esp_des), /* must write to memory, or halt() will not get it */      \
      "=m" (ret)                                                                      \
    : "r" (esp), "r" (eip)                                                    \
    : "cc", "memory"                                                                  \
)

/**
 * load_esp_and_return
 * Description: reload esp then ret to the address on kernel stack
 * Input: new_esp - the stack we want to switch to
 *        status - value to load in eax
 * Output: None
 * Side effect: Switch kernel stack
 */
#define load_esp_and_return(new_esp,status) asm volatile ("                  \
        movl %0, %%esp  /* load old ESP */                                           \n\
        ret  /* now it's equivalent to jump execute return */"                         \
        :                                                                              \
        : "r" (new_esp)   , "a" (status)                                         \
        : "cc", "memory"                                                               \
    )

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
 * get_another_terminal_id
 * Description: get the terminal id that is not empty
 * Input: id -- the empty terminal id
 * Output: None
 */
int get_another_terminal_id(int id) {
    int i;
    for (i = 0; i < MAX_TERMINAL; i++)
        if (foreground_task[i] != NULL && id!=i) return id;
    return -1;
}
/**
 * sys_execute
 * Description: execute the command
 * Input: command -- string to be executed
 *        wait_for_child -- if set to 1, means it waits for the child to return
 *                          if set to 0, indicate it's an init task
 *        separate_terminal -- if set to 1 means new terminal is used for this process
 *        function_address -- NULL means execute by args, else means execute kernel thread
 * Output: None
 * Side effect: Run the new program with the command
 */

int32_t sys_execute(uint8_t *command, int wait_for_child, int separate_terminal, void (*function_address)()) {
    pcb_t *process;
    uint32_t eip;
    uint32_t length;
    int pid_ret;
    int ret;

    // bad input
    if (command == NULL) {
        printf("ERROR in sys_execute: command NULL pointer\n");
        return -1;
    }
    // set up pcb for the new task
    process = create_process();
    if (process==NULL) return -1; // Raise Error
    process->having_child_running=0;
    process->rtc_id = -1;
    process->init_task=process->kernel_task=process->own_terminal=process->wait_for_child=0;
    // Information Setting
    if (get_n_present_pcb()==1) {
        process->init_task = 1;
        process->parent = NULL;
    } else {
        process->parent = (wait_for_child==1)?get_cur_process():NULL;
        if (wait_for_child==1) get_cur_process()->having_child_running=1;
    }
    if (function_address!=NULL) process->kernel_task=1;
    process->k_esp_base = (uint32_t)process+TASK_KSTK_SIZE_IN_B-1;
    process->k_esp = (uint32_t)process + TASK_KSTK_SIZE_IN_B-1;
    // Parse name and arguments
    parse_args(command, &(process->args));
    process->name = command;
    // move name and argument to new program's PCB
    length = strlen((int8_t *)process -> name);
    process->k_esp_base-=length;
    process->name=(uint8_t *)strcpy((int8_t *)process->k_esp_base, (int8_t *) process->name);
    if (process->args!=NULL) {
        length = strlen((int8_t *)process -> args);
        process->k_esp_base-=length;
        process->args=(uint8_t *)strcpy((int8_t *)process->k_esp_base, (int8_t *) process->args);
    }
    process -> k_esp = process -> k_esp_base;

    //  set video memory for the current process
    if (process->kernel_task) {
        process -> terminal = &null_terminal;
        process -> pid = -1; // kernel task has no paging and terminal
        eip = (uint32_t) function_address;
    }  else {
        if (separate_terminal) {
            process->terminal = terminal_allocate();
            process->own_terminal = 1;
        } else {
            if (process->init_task) process->terminal = &null_terminal;
            else process->terminal = get_cur_process()->terminal; // Use the caller's terminal
        }
        pid_ret = set_page_for_task(command, (uint32_t *)&eip);
        if (pid_ret < 0) {
            if (wait_for_child==1) get_cur_process()->having_child_running=0;
            delete_process(process);
            return -1;
        }
        process->pid = pid_ret;
        if (process->terminal != &null_terminal) {
            foreground_task[process->terminal->id] = process; // Become the task using terminal
            set_showing_task(process);
        }
        terminal_set_running(process->terminal);
        if (separate_terminal) {
            clear();
            reset_screen();
            printf("TERMINAL %d\n", process->terminal->id);
        }
    }
    init_file_arr(&(process->file_arr));
    task_signal_init(&(process->signals));
    // Set up Scheduler
    if (add_task_to_list(process)==-1) return -1;
    init_process_time(process);
    // Set up tss to make sure system call don't go wrong
    tss.esp0 = process->k_esp;
    if (process->kernel_task) {
        if (process->init_task) {
            execute_program_in_kernel_asm(length, process->k_esp, eip, ret);
        } else {
            execute_program_in_kernel_asm(get_cur_process()->k_esp, process->k_esp, eip, ret);
        }
    } else {
        if (process->init_task) {
            execute_user_program_asm(length, US_STARTING, eip, ret);
        } else {
            execute_user_program_asm(get_cur_process()->k_esp, US_STARTING, eip, ret);
        }
    }
    return ret;
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


    pcb_t * cur_task = get_cur_process();

    if(cur_task == NULL) return -1;

    // remove cur_task from task list
    delete_task_from_list(cur_task);

    // if cur_task has parent, we simply switch to parent task
    if (cur_task->parent != NULL) {
        cur_task->parent->having_child_running = 0;
        init_process_time(cur_task->parent);
    }

    // close file array
    // ---------------------------------------
    int term_id = cur_task->terminal->id;
    // Perform the deallocation of terminal
    if (cur_task->own_terminal) {
        foreground_task[term_id] = NULL;
        if (get_showing_task()->terminal->id == term_id) {
            int new_term_id = get_another_terminal_id(term_id);
            if (new_term_id != -1) {
                set_showing_task(foreground_task[new_term_id]);
            }
        }
        terminal_deallocate(cur_task->terminal);
    } else {
        foreground_task[cur_task->parent->terminal->id] = cur_task->parent;
        if (get_showing_task()->terminal->id == cur_task->parent->terminal->id) {
            set_showing_task(cur_task->parent);
        }
    }

    // ---------------------------------------


    // no parent, we just reschedule
    if(cur_task->parent == NULL){
        close_all_files(&cur_task->file_arr);
        delete_process(cur_task);
        delete_paging(cur_task->pid);
        reschedule();
    } else {
            close_all_files(&cur_task->file_arr);
            delete_process(cur_task);
            // we have parent, so just switch to them
            restore_paging(cur_task->pid,cur_task->parent->pid);
            terminal_set_running(cur_task->parent->terminal);
            tss.esp0 = cur_task->parent->k_esp_base;
            load_esp_and_return(cur_task->parent->k_esp, status);
        }

        printf("In system_halt: this function should never return.\n");
        return -1;
    }

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
    for (i = 0; i < MAX_TERMINAL; ++i) foreground_task[i] = NULL;
    init_scheduler();
}

void run_initial_task() {
    uint32_t flags;
    cli_and_save(flags);
    {
        sys_execute((uint8_t *) "initial", 0, 0, init_task_main);
    }
    restore_flags(flags);
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
 * get_n_present_pcb
 * Description: get the number of current present process
 * Input: None
 * Output: n_present_pcb
 * Side effect: None
 */
int32_t get_n_present_pcb() {
    return n_present_pcb;
}


void mouse_process(){
    mouse_init();
    enable_irq(12);
}



void update_screen() {
    uint32_t flags;
    int i,j;
    cli_and_save(flags);
    if (need_redraw_background) {

        for(i = 0; i < VGA_DIMX; i++)
            for(j = 0; j < VGA_DIMY; j++)
                Pdraw(i, j, (i-300)*(i-300)+(i-300)*(j-400)+(j-400)*(j-400)+0xEE0000);
        setup_status_bar();
    }
    char s[10] = "fish";
    if(showing_term->id != -1){
        // set the real page table to visit, or in the backup table there is no buffer (page fault)
        PDE_4K_set(&(page_directory[0]), (uint32_t) (page_table0), 1, 1, 1);
        flush_tlb();
        if (!init) {
            update_priority(0); update_priority(1); update_priority(2);
            draw_terminal((char*)(VM_BUF_SVGA_ADDR + 0 * SIZE_4K_IN_BYTES),0,0);
            draw_terminal((char*)(VM_BUF_SVGA_ADDR + 1 * SIZE_4K_IN_BYTES),1,0);
            draw_terminal((char*)(VM_BUF_SVGA_ADDR + 2 * SIZE_4K_IN_BYTES),2,1);
            init = 1;
        } else if (new_content || need_redraw_background || need_change_focus || strcmp(s, (int8_t*)get_cur_process()->name)==1) {
            for (i=0;i<=2;++i)
                for (j=0;j<3;++j)
                    if (terminal_window[j].priority==i && terminal_window[j].active==1)
                        draw_terminal((char*)(VM_BUF_SVGA_ADDR + j * SIZE_4K_IN_BYTES),j,0);
            for (j=0;j<3;++j)
                if (terminal_window[j].priority==3 && terminal_window[j].active==1)
                    draw_terminal((char*)(VM_BUF_SVGA_ADDR + j * SIZE_4K_IN_BYTES),j,1);
            new_content = 0;
        }

        terminal_vidmap_SVGA(running_term);
    }
    need_redraw_background = 0;
    need_change_focus = 0;
    restore_flags(flags);

}
/**
 * init_task_main
 * Description: Open three terminal and keep them
 * Input: None
 * Output: none
 */
void init_task_main() {

    int32_t i;
    play_wav(0);
    sys_execute((uint8_t *) "shell", 0  , 1, NULL);
    sys_execute((uint8_t *) "shell", 0, 1, NULL);
    sys_execute((uint8_t *) "shell", 0, 1, NULL);
    all_terminal_is_on = 1;
    uint32_t flags;
    while(1) {
        if (need_play) {
            cli_and_save(flags);
            sys_execute((uint8_t *) "wav_player YoungForYou.wav",1,0,NULL);
            restore_flags(flags);
            need_play = 0;
        }
        for (i = 0; i<MAX_TERMINAL;++i) if (foreground_task[i]==NULL) {
                sys_execute((uint8_t *) "shell", 0, 1, NULL);
        }
    };

}


