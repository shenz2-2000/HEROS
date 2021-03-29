#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "rtc.h"
#include "terminal.h"

#define RUN_TESTS
/* Declaration of constant */
// Maximum Size of keyboard buffer should be 128
static int key_buf_cnt=0;
static char keyboard_buf[KEYBOARD_BUF_SIZE];
static int flag[128];       // There're at most 128 characters on the keyboard
// check whether terminal_read is working
static int run_read=0;
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
/*
 * keyboard_interrupt
 *   DESCRIPTION: Handle the interrupt from keyboard
 *   INPUTS: scan code from port 0x60
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: interrupt
 */
void keyboard_interrupt_handler() {
    cli();
    uint8_t input = inb(KEYBOARD_PORT);
    int capital, letter, shift_on;
    char chr;
    if (input > KEY_BOARD_PRESSED) { // If it is a keyboard release signal
        flag[input - RELEASE_DIFF] = 0; 
        if (input - RELEASE_DIFF == CAPLCL_PRESSED) CUR_CAP^=1;
    }
    else {
        // Mark the input signal and adjust the captial state
        flag[input] = 1;
        capital = CUR_CAP ^ (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
        if (flag[CTRL_PRESSED]&&flag[L_PRESSED]) { // Manage Ctrl+l
            clear();
            reset_screen();
        } else if (scan_code_table[input]) {
            // Manage the overflow issue
            if (key_buf_cnt < 127) {
                letter = (input>=0x10&&input<=0x19) | (input>=0x1E&&input<=0x26) | (input>=0x2C&&input<=0x32);
                if (letter) {
                    chr = capital?shift_scan_code_table[input]:scan_code_table[input];
                    keyboard_buf[key_buf_cnt++]=chr;
                    putc(chr);
                } else {
                    shift_on = (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
                    chr = shift_on?shift_scan_code_table[input]:scan_code_table[input]; 
                    putc(chr);
                    keyboard_buf[key_buf_cnt++]=chr;
                }
            }
            // Handle the enter pressed
            if (input == 0x1C) {
                if (key_buf_cnt==127) putc(scan_code_table[input]), keyboard_buf[key_buf_cnt++] = scan_code_table[input];
                if (run_read) {
                    sti();
                    return;
                }else {
                    key_buf_cnt = 0;
                }
            }
        } else if (flag[BACKSPACE_PRESSED]) {  // Manage backspace
            if (key_buf_cnt) delete_last(),--key_buf_cnt;
        }

    }
        
    sti();
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
    key_buf_cnt = 0;
    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        flag[i] = 0;
        keyboard_buf[i] = 0;
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
    printf("current key_buf_cnt is: %d\n",key_buf_cnt);
    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        if(flag[i] != 0) {
            fail_flag = 1;
            break;
        }
    }

    if(fail_flag == 1) printf("The flag array is wrong!!\n");
    else printf("The flag array is correct!!\n");

    fail_flag = 0;

    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        if(keyboard_buf[i] != 0) {
            fail_flag = 1;
            break;
        }
    }

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
    terminal_initialization();
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
    int delete_length = -1;

    // invalid nbytes
    if(nbytes < 0) return -1;
    // meaningless nbytes
    if(nbytes == 0) return 0;
    // overflow nbytes
    if(nbytes > KEYBOARD_BUF_SIZE) nbytes = KEYBOARD_BUF_SIZE;

    run_read = 1;
    key_buf_cnt = 0;

    // loop until exit
    while(!end_flag){
        // prevent interrupt from modifying the line buffer
        cli();
        // detect enter
        for(i = 0; i <= (key_buf_cnt - 1) ; i++){
            if(keyboard_buf[i] == '\n'){
                j = i;
                end_flag = 1;
                break;
            }
        }

        sti();
    }

    // new critical section
    cli();
    if(j > nbytes) j = nbytes;
    if(keyboard_buf[j-1] != '\n'){
        keyboard_buf[j] = '\n';
        j = j + 1;
    }
    // copy to user buf
    memcpy(buf,keyboard_buf,j);

    // set delete_length to handle two conditions of \n and no \n
    key_buf_cnt = 0;

    run_read = 0;

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









