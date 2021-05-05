#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "rtc.h"
#include "link.h"
#include "idt.h"
#include "process.h"
#include "scheduler.h"
#include "page_lib.h"
#define RUN_TESTS
/* Declaration of constant */
// Maximum Size of keyboard buffer should be 128
terminal_struct_t null_terminal ={
        .valid = 1,
        .id = -1,
        .buf_cnt = 0,
        .screen_x = 0,
        .screen_y = 0
};
static int flag[128];       // There're at most 128 characters on the keyboard
// check whether terminal_read is working
// Using scan code set 1 for we use "US QWERTY" keyboard
// The table transform scan code to the echo character
static int CUR_CAP = 0; // Keep the current state of caplock
static const char scan_code_table[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,      /* 0x00 - 0x0E */
        ' ', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',      /* 0x0F - 0x1C */
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',           /* 0x1D - 0x29 */
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0,          /* 0x2A - 0x37 */
        0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x38 - 0x46 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0             /* 0x54 - 0x80 */
};
// The shift_scan_code_table should store the keycode after transform
static const char shift_scan_code_table[128] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',      /* 0x00 - 0x0E */
        ' ', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',      /* 0x0F - 0x1C */
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',           /* 0x1D - 0x29 */
        0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0,          /* 0x2A - 0x37 */
        0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x38 - 0x46 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0             /* 0x54 - 0x80 */
};
task_node local_wait_list = {&(local_wait_list), &(local_wait_list), NULL ,1};
terminal_struct_t terminal_array[MAX_TERMINAL]; // Maximal terminal number is 3

/* global var in vidmap.c */
// terminal
int32_t terminal_status[MAX_TERMINAL];          // the status of terminal vidmap (default all open)
static terminal_struct_t *terminal_showing;     // current showing terminal
static terminal_struct_t *terminal_running;     // the terminal that is occupied by a running process

// backup buffer
PTE k_bb_pt_0[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE k_bb_pt_1[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE k_bb_pt_2[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE* k_bb_pt_list[MAX_TERMINAL] = {k_bb_pt_0, k_bb_pt_1, k_bb_pt_2};
// NOTE!!!: the element in this list is pointers to the tables! no need to dereference again!

PTE u_bb_pt_0[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE u_bb_pt_1[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE u_bb_pt_2[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE* u_bb_pt_list[MAX_TERMINAL] = {u_bb_pt_0, u_bb_pt_1, u_bb_pt_2};
/*
 * get_showing_terminal
 *   DESCRIPTION: Return the terminal that is currently showing
 *   OUTPUTS: none
 *   RETURN VALUE: terminal_showing
 */
terminal_struct_t* get_showing_terminal(){
    return terminal_showing;
}

/*
 * handle_input
 *   DESCRIPTION: Based on the keyboard input, do the corresponding operation
 *   INPUTS: input -- the input ASCII number
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */
void handle_input(uint8_t input) {
    int capital, letter, shift_on;
    char chr;
    if (input > KEY_BOARD_PRESSED) { // If it is a keyboard release signal
        flag[input - RELEASE_DIFF] = 0;
        if (input - RELEASE_DIFF == CAPLCL_PRESSED) CUR_CAP^=1;
    }
    else {
        // Mark the input signal and adjust the capital state
        flag[input] = 1;
        capital = CUR_CAP ^ (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
        if (flag[CTRL_PRESSED]&&flag[L_PRESSED]) { // Manage Ctrl+l
            clear();
            reset_screen();
        } else if (scan_code_table[input]) {
            // Manage the overflow issue
            if (flag[CTRL_PRESSED] && flag[C_PRESSED]) {
                get_showing_task()->terminal->buf_cnt = 0;
                signal_send(2); // Interrupt
                return;
            }
            if (get_showing_task()->terminal->buf_cnt < KEYBOARD_BUF_SIZE - 1) {
                letter = (input>=0x10&&input<=0x19) | (input>=0x1E&&input<=0x26) | (input>=0x2C&&input<=0x32);   // Check whether the input is letter
                if (letter) {
                    chr = capital?shift_scan_code_table[input]:scan_code_table[input];
                    get_showing_task()->terminal->buf[get_showing_task()->terminal->buf_cnt++]=chr;
                    putc(chr);
                } else {
                    shift_on = (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
                    chr = shift_on?shift_scan_code_table[input]:scan_code_table[input];
                    putc(chr);
                    get_showing_task()->terminal->buf[get_showing_task()->terminal->buf_cnt++]=chr;
                }
            }
            // Handle the enter pressed and reading is turned on
            if ((get_showing_task()->terminal->user_ask)!=0 && input == 0x1C) {    // If enter is pressed
                get_showing_task()->terminal->buf[get_showing_task()->terminal->buf_cnt] = '\n';
//                putc(get_showing_task()->terminal->buf[get_showing_task()->terminal->buf_cnt]);
                get_showing_task()->terminal->buf_cnt++;
                return;
            }
            if ((0 == get_showing_task()->terminal->user_ask) && input == 0x1C) {
                get_showing_task()->terminal->buf_cnt = 0;
                putc('\n');
                return;
            }
        } else if (flag[BACKSPACE_PRESSED]) {  // Manage backspace
            if (get_showing_task()->terminal->buf_cnt) delete_last(),--get_showing_task()->terminal->buf_cnt;
        } else if (flag[ALT_PRESSED]) {
            if (flag[F1_PRESSED]) change_focus_terminal(0);
            else if (flag[F2_PRESSED]) change_focus_terminal(1);
            else if (flag[F3_PRESSED]) change_focus_terminal(2);
        }
    }
}
/*
 * keyboard_interrupt
 *   DESCRIPTION: Handle the interrupt from keyboard
 *   INPUTS: scan code from port 0x60
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: interrupt
 */
ASMLINKAGE void keyboard_interrupt_handler(hw_context hw) {

    uint8_t input = inb(KEYBOARD_PORT);
    uint32_t flags;
    send_eoi(hw.irq);
    cli_and_save(flags);
    {

        if (get_showing_task()) {
            terminal_set_running(get_showing_task()->terminal);
        }
    }
    handle_input(input);
    terminal_set_running(get_cur_process()->terminal);
    restore_flags(flags);
}


/*
 * terminal_initialization
 *   DESCRIPTION: Initialize the terminal
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modify flag and line buffer
 */
void terminal_initialization(){
    int i;
    // initialization
    for(i = 0; i < MAX_TERMINAL; i++){
        terminal_array[i].valid = 0;
    }
}

/*
 * terminal_open
 *   DESCRIPTION: Open the terminal
 *   INPUTS: filename -- name of file
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: modify flag and line buffer
 */
int32_t terminal_open(const uint8_t* filename){
    // initialization
    return 0;
}

/*
 * terminal_close
 *   DESCRIPTION: Close the terminal
 *   INPUTS: fd -- file descriptor of terminal
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t terminal_close(int32_t fd){
    // close
    return 0;
}


/*
 * terminal_read
 *   DESCRIPTION: Read from line buffer to user buffer
 *   INPUTS: fd -- file descriptor of terminal
 *           buf -- user buffer pointer
 *           nbytes-- number of bytes user want
 *   OUTPUTS: 0 -- success
 *            -1 -- fail
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes){

    int i,j;
    int end_flag = 0;
    uint32_t  flags;
    // invalid nbytes
    if(nbytes < 0) return -1;
    // meaningless nbytes
    if(nbytes == 0) return 0;
    // overflow nbytes
    if(nbytes >= KEYBOARD_BUF_SIZE) nbytes = KEYBOARD_BUF_SIZE - 1;

    // change the status
    get_cur_process()->terminal->user_ask = nbytes;
    // clear the line buffer

    // loop until exit
    while(!end_flag) {
        // prevent interrupt from modifying the line buffer
        cli_and_save(flags);
        // detect enter
        for (i = 0; i <= (get_cur_process()->terminal->buf_cnt - 1); i++) {
            if (get_cur_process()->terminal->buf[i] == '\n') {
                j = i;
                end_flag = 1;
                break;
            }
        }
        restore_flags(flags);
    }
    // printf("Getting out of loop\n");
    get_cur_process()->terminal->user_ask = 0;
    // new critical section
    cli_and_save(flags);
    // set the copy number
    if(j > nbytes) j = nbytes;
    if(get_cur_process()->terminal->buf[j-1] != '\n'){
        get_cur_process()->terminal->buf[j] = '\n';
        j = j + 1;
    }
    // copy to user buf
    memcpy(buf,get_cur_process()->terminal->buf,j);

    // set delete_length to handle two conditions of \n and no \n
    get_cur_process()->terminal->buf_cnt = 0;


    restore_flags(flags);



    return j;

}


/*
 * terminal_write
 *   DESCRIPTION: Write from user buffer to screen
 *   INPUTS: fd -- file descriptor of terminal
 *           buf -- user buffer pointer
 *           nbytes-- number of bytes user want
 *   OUTPUTS: 0 -- success
 *            -1 -- fail
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
int32_t  terminal_write(int32_t fd, const void* buf, int32_t nbytes){

    int i;
    int flags;

    // sanity check
    if (buf == NULL) {
        printf("ERROR in terminal_write(): NULL buf input\n");
        return -1;
    }

    // prevent rtc or other interrupt to disrupt
    cli_and_save(flags);

//    printf("WRITE:Current Running Terminal:%d\n",get_running_terminal()->id);
//    printf("Before: %d %d\n",get_running_terminal()->screen_x,get_running_terminal()->screen_y);
    for(i = 0; i < nbytes; i++){
        putc( ((uint8_t*) buf)[i] );
    }
//    printf("After: %d %d\n",get_running_terminal()->screen_x,get_running_terminal()->screen_y);

    restore_flags(flags);
    return 0;

}


/*
 * terminal_init
 *   DESCRIPTION: Initialize the terminal slot
 *   INPUTS: None
 *   OUTPUTS: None
 *   RETURN VALUE: none
 *   SIDE EFFECTS: all valid field is set to 0
 */

void terminal_init() {
    int i;
    for (i = 0; i < MAX_TERMINAL; ++i) terminal_array[i].valid = 0;
}

/*
 * terminal_deallocate
 *   DESCRIPTION: deallocate current terminal
 *   INPUTS: cur -- pointer to current terminal
 *   OUTPUTS: None
 *   RETURN VALUE: none
 */

void terminal_deallocate(terminal_struct_t* cur) {
    // sanity check
    if (cur == NULL) {
        printf("ERROR in terminal_deallocate(): NULL cur input\n");
        return;
    }
    cur->valid = 0;
}

/*
 * terminal_allocate
 *   DESCRIPTION: allocate terminal slot for current terminal
 *   INPUTS: none
 *   OUTPUTS: pointer to the current terminal
 *   RETURN VALUE: none
 */

terminal_struct_t* terminal_allocate() {
    int x;
    for (x = 0; x < MAX_TERMINAL; ++x) if (!terminal_array[x].valid) break;
    if (x>=MAX_TERMINAL) {
        printf("NOT AVAILABLE TERMINAL SLOT\n");
        return NULL;
    }
    terminal_array[x].valid = 1;
    terminal_array[x].id = x;
    terminal_array[x].buf_cnt = 0;
    terminal_array[x].screen_x = terminal_array[x].screen_y = 0;
    return &terminal_array[x];
}

/**
 * terminal_set_running
 * Description: map physical video memory
 * Input: terminal -- pointer to the terminal
 * Output: None
 * Side effect: None
 */

void terminal_set_running(terminal_struct_t *terminal) {
    // sanity check
    if (get_running_terminal() == NULL) {
//        printf("ERROR for terminal_set_running(): running_terminal is NULL\n");
        return;
    }
    if (terminal == NULL) {
//        printf("ERROR in terminal_set_running(): NULL terminal input\n");
        return;
    }
    uint32_t flags;
    cli_and_save(flags);
    terminal_struct_t* cur = get_running_terminal();
    if (terminal == cur) {
        restore_flags(flags);
        return;
    }
    cur -> screen_x = screen_x;
    cur -> screen_y = screen_y;
    terminal_vidmap_SVGA(terminal);
    screen_x = terminal->screen_x;
    screen_y = terminal->screen_y;
    set_running_terminal(terminal);
    restore_flags(flags);
}

/* ----------------------------- vidmap ----------------------------- */
/* vidmap_init
 * Description: initialize the video memory map
 * Inputs: None
 * Return Value: None
 * Side effect: initialize the video memory map
 * */
void vidmap_init() {
    int i, j;

    terminal_showing = &null_terminal;
    terminal_running = &null_terminal;

    for (i = 0; i < MAX_TERMINAL; i++) {
        terminal_status[i] = TERMINAL_OFF;

        /* set the backup buffers for kernel and user vid map */

        // firstly clear the back up buffers
        for (j = 0; j < PAGE_TABLE_SIZE; j++) {     // NOTE!!!: the size is 1024 not 4096! (4 byte)
            k_bb_pt_list[i][j].val = 0;
            u_bb_pt_list[i][j].val = 0;
        }

        // map the 0xB8000 of user and kernel backup buffers to backup area
        // which is 0xF0000000 + i * 0x1000
        PTE_set(&(k_bb_pt_list[i][VM_PTE]), VM_BUF_SVGA_ADDR + i * BITS_4K, 0, 1, 1);
        PTE_set(&(u_bb_pt_list[i][VM_PTE]), VM_BUF_SVGA_ADDR + i * BITS_4K, 1, 1, 1);

        // map to the kernel page itself
         page_table0[VM_BUF_SVGA_PT_INDEX + i] = k_bb_pt_list[i][VM_PTE];

        // turn on all the terminals
        terminal_status[i] = TERMINAL_ON; // the terminals are definitely on
    }

    flush_tlb();
}

/* set_video_memory
 * Description: setup page directory for user manipulation
 * Inputs: None
 * Return Value: None
 * Side effect: None
 * */
void set_video_memory(terminal_struct_t *terminal){

    // sanity check
    if (terminal == NULL) {
        printf("ERROR in set_video_memory(): NULL terminal input\n");
        return;
    }

    // the page directory entry we need to setup
    PDE *u_vm_pde = &(page_directory[U_VM_PDE]);

    if (terminal == &null_terminal) {
        // turn off the user vidmap
        u_vm_pde->val = 0;
    } else {
        if (terminal == terminal_showing) {
            // make the memory map show
            PDE_4K_set(u_vm_pde, (uint32_t) (video_page_table0), 1, 1, 1);    // user level
        } else {
            // map the video mem to backup buffers
            PDE_4K_set(u_vm_pde, (uint32_t) (u_bb_pt_list[terminal->id]), 1, 1, 1);
        }
    }

    flush_tlb();
}

/* set_video_memory_SVGA
 * Description: setup page directory for user manipulation for SVGA mode (everything to buffer)
 * Inputs: terminal - the desired terminal
 * Return Value: None
 * Side effect: None
 * */
void set_video_memory_SVGA(terminal_struct_t *terminal){

    // sanity check
    if (terminal == NULL) {
        printf("ERROR in set_video_memory(): NULL terminal input\n");
        return;
    }

    // the page directory entry we need to setup
    PDE *u_vm_pde = &(page_directory[U_VM_PDE]);

    if (terminal == &null_terminal) {
        // turn off the user vidmap
        u_vm_pde->val = 0;
    } else {
        // no matter what, always map the video mem to backup buffers
        PDE_4K_set(u_vm_pde, (uint32_t) (u_bb_pt_list[terminal->id]), 1, 1, 1);
    }

    flush_tlb();
}


/* ------------------- terminal operation --------------------- */


/* swtich_terminal
 * Description: switch the terminal
 * Inputs: old_terminal - the old shown terminal(to be stored). this param can be NULL because the
 *                   process can allocate a terminal from nothing
 *         new_terminal - the new terminal to be shown (this param can also be NULL)
 * Return Value: None
 * Side effect: None
 * */
int switch_terminal(terminal_struct_t *old_terminal, terminal_struct_t *new_terminal) {
    // sanity check
    if (old_terminal == NULL) {
        printf("ERROR in switch_terminal(): NULL old_terminal input\n");
        return -1;
    }
    if (new_terminal == NULL) {
        printf("ERROR in switch_terminal(): NULL new_terminal input\n");
        return -1;
    }

    if (old_terminal == new_terminal) return 0;
    if (old_terminal != &null_terminal) {
        if (old_terminal->id < 0 || old_terminal->id >= MAX_TERMINAL) {
            printf("ERROR in swtich_terminal(): bad input of old_terminal\n");
            return -1;
        }
        if (terminal_status[old_terminal->id] == TERMINAL_OFF) {
            printf("ERROR in swtich_terminal(): old_terminal not turned on\n");
            return -1;
        }
    }
    if (new_terminal != &null_terminal) {
        if (new_terminal->id < 0 || new_terminal->id >= MAX_TERMINAL) {
            printf("ERROR in swtich_terminal(): bad input of new_terminal\n");
            return -1;
        }
        if (terminal_status[new_terminal->id] == TERMINAL_OFF) {
            printf("ERROR in swtich_terminal(): new_terminal not turned on\n");
            return -1;
        }
    }

    // set the real page table to switch, or in the backup table there is no buffer (page fault)
    int ret;
    ret = PDE_4K_set(&(page_directory[0]), (uint32_t) &(page_table0), 0, 1, 1);
    if (ret == -1) {
        printf("ERROR in switch: PDE set wrong\n");
        return -1;
    }
    flush_tlb();

    // copy the physical video memory to the backup buffer
    if (old_terminal != &null_terminal) {
        memcpy((uint8_t *) (VM_INDEX + (old_terminal->id + 1) * BITS_4K), (uint8_t *) (VM_INDEX), BITS_4K);
    }

    // copy the backup buffer of new terminal to the video memory area
    if (new_terminal != &null_terminal) {
        memcpy((uint8_t *) (VM_INDEX), (uint8_t *) (VM_INDEX + (new_terminal->id + 1) * BITS_4K), BITS_4K);
    }

    terminal_showing = new_terminal;
    terminal_vidmap(terminal_running);

    return 0;
}

/* terminal_vidmap
 * Description: map the target terminal to the corresponding memory area (video mem or backup buffer)
 * Inputs: terminal - the target terminal
 * Return Value: 0 for success
 * Side effect: None
 * */
/* NOTE: this function is only invoked when set running terminal (reschedule) and switch terminal*/
int terminal_vidmap(terminal_struct_t *terminal) {
    int ret;

    // sanity check
    if (terminal == NULL) {
        printf("ERROR in terminal_vidmap(): NULL terminal input\n");
        return -1;
    }

    // sanity check
    if (terminal != &null_terminal) {
        if (terminal->id < 0 || terminal->id >= MAX_TERMINAL) {
            printf("ERROR in terminal_vidmap(): bad input of terminal\n");
            return -1;
        }
        if (terminal_status[terminal->id] == TERMINAL_OFF) {
            printf("ERROR in terminal_vidmap(): terminal not turned on\n");
            return -1;
        }
    }

    if (terminal == &null_terminal || (terminal != &null_terminal && terminal == terminal_showing)) {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) (page_table0), 0, 1, 1);
        if (ret == -1) return -1;
    } else {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) (k_bb_pt_list[terminal->id]), 0, 1, 1);
        if (ret == -1) return -1;
    }
    terminal_running = terminal;
    flush_tlb();
    return 0;
}

/* terminal_vidmap_SVGA
 * Description: map the target terminal to the corresponding memory area (video mem or backup buffer) (for SVGA)
 * Inputs: terminal - the target terminal
 * Return Value: 0 for success
 * Side effect: None
 * */
/* NOTE: this function is only invoked when set running terminal (reschedule) and switch terminal*/
int terminal_vidmap_SVGA(terminal_struct_t *terminal) {
    int ret;

    // sanity check
    if (terminal == NULL) {
        printf("ERROR in terminal_vidmap(): NULL terminal input\n");
        return -1;
    }

    // sanity check
    if (terminal != &null_terminal) {
        if (terminal->id < 0 || terminal->id >= MAX_TERMINAL) {
            printf("ERROR in terminal_vidmap(): bad input of terminal\n");
            return -1;
        }
        if (terminal_status[terminal->id] == TERMINAL_OFF) {
            printf("ERROR in terminal_vidmap(): terminal not turned on\n");
            return -1;
        }
    }

    if (terminal == &null_terminal) {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) (k_bb_pt_list[0]), 0, 1, 1);
        if (ret == -1) return -1;
    } else {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) (k_bb_pt_list[terminal->id]), 0, 1, 1);
        if (ret == -1) return -1;
    }
    terminal_running = terminal;
    flush_tlb();
    return 0;
}
