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
#include "vidmap.h"
#include "page_lib.h"
#define RUN_TESTS
/* Declaration of constant */
// Maximum Size of keyboard buffer should be 128
static int key_buf_cnt=0;
static char keyboard_buf[KEYBOARD_BUF_SIZE];
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
terminal_struct_t terminal_slot[MAX_TERMINAL]; // Maximal terminal number is 3

// global var in vidmap.c
extern int32_t terminal_status[MAX_TERMINAL];
extern terminal_struct_t *terminal_showing;     // current showing terminal
extern terminal_struct_t *terminal_running;     // the terminal that is occupied by a running process
extern PTE k_bb_pt_list[MAX_TERMINAL][PAGE_TABLE_SIZE];
extern PTE u_bb_pt_list[MAX_TERMINAL][PAGE_TABLE_SIZE];

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
            if (focus_task()->terminal->buf_cnt < KEYBOARD_BUF_SIZE - 1) {
                letter = (input>=0x10&&input<=0x19) | (input>=0x1E&&input<=0x26) | (input>=0x2C&&input<=0x32);   // Check whether the input is letter
                if (letter) {
                    chr = capital?shift_scan_code_table[input]:scan_code_table[input];
                    focus_task()->terminal->buf[focus_task()->terminal->buf_cnt++]=chr;
                    putc(chr);
                } else {
                    shift_on = (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
                    chr = shift_on?shift_scan_code_table[input]:scan_code_table[input];
                    putc(chr);
                    focus_task()->terminal->buf[focus_task()->terminal->buf_cnt++]=chr;
                }
            }
            // Handle the enter pressed and reading is turned on
            if ((focus_task()->terminal->user_ask)!=0 && input == 0x1C) {    // If enter is pressed
                focus_task()->terminal->buf[focus_task()->terminal->buf_cnt] = '\n';
                putc(focus_task()->terminal->buf[focus_task()->terminal->buf_cnt]);
                focus_task()->terminal->buf_cnt++;
                //focus_task()->parent->flags &= ~TASK_WAITING_CHILD;
                init_process_time(focus_task());
                insert_to_list_start(focus_task()->node);
                reschedule();
                return;
            }
            if ((0 == focus_task()->terminal->user_ask) && input == 0x1C) {
                focus_task()->terminal->buf_cnt = 0;
                putc('\n');
                return;
            }
        } else if (flag[BACKSPACE_PRESSED]) {  // Manage backspace
            if (focus_task()->terminal->buf_cnt) delete_last(),--focus_task()->terminal->buf_cnt;
        } else if (flag[ALT_PRESSED]) {
            if (flag[F1_PRESSED]) change_focus_task(1);
            else if (flag[F2_PRESSED]) change_focus_task(2);
            else if (flag[F3_PRESSED]) change_focus_task(3);
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
    cli_and_save(flags);
    {
        send_eoi(hw.irq);
        if (focus_task()) {
            terminal_set_running(focus_task()->terminal);
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
        terminal_slot[i].valid = 0;
    }
}

/*
 * print_terminal_info
 *   DESCRIPTION: For test, print some sentences
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
void print_terminal_info(){
    int i;
    int fail_flag = 0;
    // print current key buf cnt
    printf("current key_buf_cnt is: %d\n",key_buf_cnt);
    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        if(flag[i] != 0) {
            fail_flag = 1;
            break;
        }
    }

    // check fail_flag
    if(fail_flag == 1) printf("The flag array is wrong!!\n");
    else printf("The flag array is correct!!\n");

    fail_flag = 0;

    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        if(keyboard_buf[i] != 0) {
            fail_flag = 1;
            break;
        }
    }

    // fail or not
    if(fail_flag == 1) printf("The keyboard buffer is wrong!!\n");
    else printf("The keyboard buffer is correct!!\n");

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
    (void)filename;
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
    (void) fd;
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
    //while(!end_flag){
        // prevent interrupt from modifying the line buffer
    cli();
    // detect enter
    for(i = 0; i <= (get_cur_process()->terminal->buf_cnt - 1) ; i++){
        if(get_cur_process()->terminal->buf[i] == '\n'){
            j = i;
            end_flag = 1;
            break;
        }
    }
    sti();
    if (!end_flag) {
//        get_cur_process()->flags |= TASK_WAITING_CHILD;
        append_to_list_end(&local_wait_list);
        reschedule();
    }
    //}
    get_cur_process()->terminal->user_ask = 0;
    // new critical section
    cli();
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


    sti();



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

    // prevent rtc or other interrupt to disrupt
    cli();

    for(i = 0; i < nbytes; i++){
        putc( ((uint8_t*) buf)[i] );
    }

    sti();
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
    for (i = 0; i < MAX_TERMINAL; ++i) terminal_slot[i].valid = 0;
}

/*
 * terminal_deallocate
 *   DESCRIPTION: deallocate current terminal
 *   INPUTS: cur -- pointer to current terminal
 *   OUTPUTS: None
 *   RETURN VALUE: none
 */

void terminal_deallocate(terminal_struct_t* cur) {
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
    for (x = 0; x < MAX_TERMINAL; ++x) if (!terminal_slot[x].valid) break;
    if (x>=MAX_TERMINAL) {
        printf("NOT AVAILABLE TERMINAL SLOT\n");
        return NULL;
    }
    terminal_slot[x].valid = 1;
    terminal_slot[x].id = x;
    terminal_slot[x].buf_cnt = 0;
    terminal_slot[x].screen_x = terminal_slot[x].screen_y = 0;
    return &terminal_slot[x];
}




/**
 * terminal_set_running
 * Description: map physical video memory
 * Input: terminal -- pointer to the terminal
 * Output: None
 * Side effect: None
 */

void terminal_set_running(terminal_struct_t *terminal) {
    if (terminal == get_running_terminal()) return ;
    get_running_terminal() -> screen_x = screen_x;
    get_running_terminal() -> screen_y = screen_y;
    terminal_vidmap(terminal);
    screen_x = terminal->screen_x;
    screen_y = terminal->screen_y;
    set_running_terminal(terminal);
}

/* ------------------- terminal operation --------------------- */

/* terminal_turn_on
 * Description: turn on the terminal
 * Inputs: terminal - the target terminal struct
 * Return Value: None
 * Side effect: None
 * */
int terminal_turn_on(terminal_struct_t *terminal) {
    if (terminal == NULL) {
        printf("ERROR in terminal_turn_on(): NULL input");
        return -1;
    }
    if (terminal->id < 0 || terminal->id >= MAX_TERMINAL) {
        printf("ERROR in terminal_turn_on(): no such terminal_id: %d", terminal->id);
        return -1;
    }
    if (terminal_status[terminal->id] == TERMINAL_ON) {
        printf("WARNING in terminal_turn_on(): the terminal %d is already turned on\n", terminal->id);
    }

    terminal_status[terminal->id] = TERMINAL_ON;
    return 0;
}

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
    if (old_terminal == new_terminal) return 0;
    if (old_terminal != NULL) {
        if (old_terminal->id < 0 || old_terminal->id >= MAX_TERMINAL) {
            printf("ERROR in swtich_terminal(): bad input of old_terminal\n");
            return -1;
        }
        if (terminal_status[old_terminal->id] == TERMINAL_OFF) {
            printf("ERROR in swtich_terminal(): old_terminal not turned on\n");
            return -1;
        }
    }
    if (new_terminal != NULL) {
        if (new_terminal->id < 0 || new_terminal->id >= MAX_TERMINAL) {
            printf("ERROR in swtich_terminal(): bad input of new_terminal\n");
            return -1;
        }
        if (terminal_status[new_terminal->id] == TERMINAL_OFF) {
            printf("ERROR in swtich_terminal(): new_terminal not turned on\n");
            return -1;
        }
    }

    // copy the physical video memory to the backup buffer
    if (old_terminal != NULL) {
        memcpy((uint8_t *) (VM_INDEX + (old_terminal->id + 1) * BITS_4K), (uint8_t *) (VM_INDEX), BITS_4K);
    }

    // copy the backup buffer of new terminal to the video memory area
    if (new_terminal != NULL) {
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
int terminal_vidmap(terminal_struct_t *terminal) {
    int ret;

    // sanity check
    if (terminal != NULL) {
        if (terminal->id < 0 || terminal->id >= MAX_TERMINAL) {
            printf("ERROR in terminal_vidmap(): bad input of terminal\n");
            return -1;
        }
        if (terminal_status[terminal->id] == TERMINAL_OFF) {
            printf("ERROR in terminal_vidmap(): terminal not turned on\n");
            return -1;
        }
    }

    if (terminal == NULL || (terminal != NULL && terminal == terminal_showing)) {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) &(page_table0), 0, 1, 1);
        if (ret == -1) return -1;
    } else {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) &(k_bb_pt_list[terminal->id]), 0, 1, 1);
        if (ret == -1) return -1;
    }
    terminal_running = terminal;
    flush_tlb();
    return 0;
}

