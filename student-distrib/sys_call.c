#include "file_sys.h"
#include "process.h"
#include "lib.h"
#include "page_lib.h"
#include "tests.h"
#include "signal_sys_call.h"
#define FILE_CLOSED_TEST 1

/**
 * sys_open
 * Description: system call: open a file
 * Input: f_name - file name
 * Output: the allocated fd
 * Side effect: the file is in use
 */
int32_t sys_open(const uint8_t *f_name) {
    dentry_t dentry;
    int32_t fd = -1;
    pcb_t *cur_pcb;
//    int i;
//    i = 1/0;

    // For error test, we can return back to shell!
//    int i = 1;
//    i = i / 0;
    cur_pcb = get_cur_process();

    if (cur_pcb->file_arr.n_opend_files >= N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_open: cannot OPEN file [%s] because the max number of files is reached\n", f_name);
        return -1;
    }

    if (f_name == NULL) {
        printf("ERROR [SYS FILE] in sys_open: f_name NULL pointer\n");
        return -1;
    }

    // obtain the dentry to know the file type
    if (read_dentry_by_name(f_name, &dentry) == -1) {
        printf("WARNING [SYS FILE] in sys_open: cannot OPEN file with name: [%s]\n", f_name);
        return -1;
    }

    // invoke the different open op according to file type
    switch (dentry.f_type) {
        case 0: {   // RTC file
            fd = file_rtc_open(f_name);
            break;
        }
        case 1: {   // directory file
            fd = dir_open(f_name);
            break;
        }
        case 2: {   // regulary file
            fd = file_open(f_name);
            break;
        }
    }

    cur_pcb->file_arr.n_opend_files++;

    return fd;
}

/**
 * sys_close
 * Description: system call: close a file
 * Input: fd - the file descriptor
 * Output: 0 if success
 * Side effect: the file is not in use
 */
int32_t sys_close(int32_t fd) {
    int32_t ret;
    pcb_t *cur_pcb;

    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_close: invalid fd\n");
        return -1;
    }
    if (fd == 0) {printf("ERROR [FILE]: cannot CLOSE stdin\n"); return -1;}
    if (fd == 1) {printf("ERROR [FILE]: cannot CLOSE stdout\n"); return -1;}

    // get the current process
    cur_pcb = get_cur_process();

    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [FILE]: cannot CLOSE a file that is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }

    ret = cur_pcb->file_arr.files[fd].f_op->close(fd);

    cur_pcb->file_arr.files[fd].flags = AVAILABLE;  // empty the block
    cur_pcb->file_arr.n_opend_files--;

    return ret;
}

/**
 * sys_read
 * Description: system call: get the content of a file
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: 0 if success
 * Side effect: the buffer will be filled
 */
int32_t sys_read(int32_t fd, void *buf, int32_t bufsize) {
    pcb_t *cur_pcb = get_cur_process();

    // bad input checking
    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_read: invalid fd\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR [SYS FILE] in sys_read: read buffer NULL pointer\n");
        return -1;
    }
    if (bufsize < 0) {
        printf("ERROR [SYS FILE] in sys_read: read buffer size should be non-negative\n");
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_read: cannot READ a file that is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }
    // stdout is write-only
    if (fd == 1) {
        printf("ERROR [SYS FILE] in sys_read: stdout is write-only\n");
        return -1;
    }

    /* for debug */
    // dentry_t dentry;
    // read_dentry_by_inode(file_arr[fd].inode_idx, &dentry);
    // printf("In file_test: Now the file type is %d\n", dentry.f_type);

    return cur_pcb->file_arr.files[fd].f_op->read(fd, buf, bufsize);
}

/**
 * invalid_sys_call
 * Description: for invalid system call number
 * Input: None
 * Output: None
 * Side effect: None
 */

void invalid_sys_call(){
     printf("The system call is invalid!!!! Please check the call number!!! \n");
}

/**
 * sys_write
 * Description: system call: write to a file (NOTE: read-only file system!)
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: -1: fail
 * Side effect: an error will be promped
 */
int32_t sys_write(int32_t fd, const void *buf, int32_t bufsize) {
    pcb_t *cur_pcb = get_cur_process();

    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_write: invalid fd\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR [SYS FILE] in sys_write: write buffer NULL pointer\n");
        return -1;
    }
    if (bufsize < 0) {
        printf("ERROR [SYS FILE] in sys_write: write buffer size should be non-negative\n");
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_write: the file is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }
    return cur_pcb->file_arr.files[fd].f_op->write(fd, buf, bufsize);
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

int32_t sys_execute(uint8_t *command) {
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
    process->parent = get_n_present_pcb()==1?NULL:get_cur_process();
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

//    // set video memory for the current process
//    set_video_memory();

    pid_ret = set_page_for_task(command, (uint32_t *)&eip);
    if (pid_ret < 0) {
        delete_process(process);
        return -1;
    }
    process->pid = pid_ret;
    init_file_arr(&(process->file_arr));
    task_signal_init(&(process->signals));
    // Set up tss to make sure system call don't go wrong
    tss.ss0 = KERNEL_DS;
    tss.esp0 = process->k_esp;

    if (process->parent==NULL) asm volatile ("                                        \
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
    ); else asm volatile ("                                                           \
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
 * system_halt
 * Description: Implementation of halt()
 * Input: status - exit status info
 * Output: 0 if success.
 * Return: should never return
 * Side effect: terminate the process and return value to its parent
 */

int32_t system_halt(int32_t status) {

    // condition check
    if (get_n_present_pcb() == 0) {   // not in a process

        // clear page directory for video memory
        clear_video_memory();
        sys_execute((uint8_t *) "shell");

        return -1;
    }
    if (get_cur_process()->parent == NULL) {
        // So the main program of base shell return on exit, so we need to execute shell again
        printf("Warning in system_halt(): halting the base shell...\n");
        close_all_files(&get_cur_process()->file_arr);  // close FDs
        delete_process(get_cur_process());
        restore_paging(get_cur_process()->pid, get_cur_process()->pid); // restore the current paging
        // CHECK ALL FILES ARE CLEANED UP
        file_closed_test();

        // clear page directory for video memory
        clear_video_memory();

        sys_execute((uint8_t *) "shell");
        return -1;
    }

    close_all_files(&get_cur_process()->file_arr);  // close FDs
    pcb_t *parent = delete_process(get_cur_process());  // clear the pcb
    restore_paging(get_cur_process()->pid, parent->pid);  // restore parent paging
    tss.esp0 = parent->k_esp;  // set tss to parent's kernel stack to make sure system calls use correct stack

    // clear page directory for video memory
    clear_video_memory();

#if FILE_CLOSED_TEST
    file_closed_test(); // CHECK ALL FILES ARE CLEANED UP
#endif

    // load esp and return
    asm volatile ("                                                                    \
        movl %0, %%esp  /* load old ESP */                                           \n\
        ret  /* now it's equivalent to jump execute return */"                         \
        :                                                                              \
        : "r" (parent->k_esp)   , "a" (status)                                         \
        : "cc", "memory"                                                               \
    );

    printf("In system_halt: this function should never return.\n");
    return -1;
}


/**
 * sys_vidmap
 * Description: send pointer to vm to user
 * Input: screen_start -- a virtual address in user region, contain a pointer to vm
 * Output: 0 - if success
 *        -1 - if fail
 * Side effect: modify the page directory
 */

int sys_vidmap(uint8_t** screen_start){

    int check_address = (int) screen_start;

    // check whether screen_start is in user region (virtual 128MB to 132 MB)
    if( (check_address < USER_VA_START) || (check_address >= USER_VA_END) ){
        printf("The screen start virtual address when you call vidmap is invalid!!\n");
        return -1;
    }

    // setup the video memory in PD
    set_video_memory();

    *screen_start = (uint8_t*)(USER_VA_END + VM_INDEX);

    return 0;
}

/**
 * sys_get_args
 * Description: get args to user
 * Input: buf -- user buffer
 *        nbytes -- num of byte
 * Output: 0 - if success
 *        -1 - if fail
 * Side effect: None
 */
int32_t sys_get_args(uint8_t *buf, int32_t nbytes){

    // sanity check
    if(buf == NULL || nbytes < 0) return -1;

    uint8_t* cur_arg = get_cur_process()->args;

    // check arg is correct
    if(cur_arg == NULL || (strlen((int8_t *) cur_arg) > nbytes)) return -1;

    // copy the arg, change to int8_t to meet the requirement of strnpy
    strncpy((int8_t *) buf, (int8_t *) cur_arg, nbytes);

    return 0;
}

/**
 * sys_play_sound
 * Description: Play sound using built in speaker
 * Input: nFrequence -- number of frequence
 * Output: always 0
 * Side effect: make sound
 */
/* reference: https://wiki.osdev.org/PC_Speaker */
/* NOTE: all the magic numbers are provided on the net and without any explanation */
int32_t sys_play_sound(uint32_t nFrequence) {
    uint32_t divide;
    uint8_t temp;

    // Set the PIT to the desired frequency
    divide = 1193180 / nFrequence; 
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t) (divide));
    outb(ox42, (uint8_t) (divide >> 8));

    // play the sound using the PC speaker
    temp = inb(0x61);
    if (temp != (temp | 3)) {
        outb(0x61, temp | 3);
    }

    return 0;
}

/**
 * sys_nosound
 * Description: stop the sound
 * Input: None
 * Output: always 0
 * Side effect: stop sound
 */
/* reference: https://wiki.osdev.org/PC_Speaker */
/* NOTE: all the magic numbers are provided on the net and without any explanation */
int32_t sys_nosound() {
    uint8_t temp = inb(0x61) & 0xFC;
    outb(0x61, temp);
    return 0;
}
