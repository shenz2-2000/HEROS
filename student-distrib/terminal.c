#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "tests.h"
#include "rtc.h"
#include "terminal.h"

#define RUN_TESTS
//void keyboard_interrupt_handler();
/* Declaration of constant */
//#define KEYBOARD_PORT 0x60
//// Maximum Size of keyboard buffer should be 128
//#define KEYBOARD_BUF_SIZE 128
static int key_buf_cnt=0;
static char keyboard_buf[KEYBOARD_BUF_SIZE];
static int flag[128]; // There're at most 128 characters on the keyboard
//#define KEY_BOARD_PRESSED 0x60
//#define RELEASE_DIFF      0x80
//#define CTRL_PRESSED      0x1D
//#define CAPLCL_PRESSED      0x3A
//#define LEFT_SHIFT_PRESSED  0x2A
//#define RIGHT_SHIFT_PRESSED 0x36
//#define L_PRESSED           0x26
//#define BACKSPACE_PRESSED   0x0E
// Using scan code set 1 for we use "US QWERTY" keyboard
// The table transform scan code to the echo character
static int CUR_CAP = 0; // Keep the current state of caplock
static const char scan_code_table[128] = {
        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0,      /* 0x00 - 0x0E */
        0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',      /* 0x0F - 0x1C */
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',           /* 0x1D - 0x29 */
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0,          /* 0x2A - 0x37 */
        0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,                            /* 0x38 - 0x46 */
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0             /* 0x54 - 0x80 */
};
// The shift_scan_code_table should store the keycode after transform
static const char shift_scan_code_table[128] = {
        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',      /* 0x00 - 0x0E */
        0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',      /* 0x0F - 0x1C */
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
    // Get input from keyboard and check whether the scan code can be output
    if (input > KEY_BOARD_PRESSED) {
        flag[input - RELEASE_DIFF] = 0; 
        if (input - RELEASE_DIFF == CAPLCL_PRESSED) CUR_CAP^=1;
    }
    else {
        flag[input] = 1;
        capital = CUR_CAP ^ (flag[LEFT_SHIFT_PRESSED]|flag[RIGHT_SHIFT_PRESSED]);
        if (flag[CTRL_PRESSED]&&flag[L_PRESSED]) {
            clear();
            reset_screen();
        } else if (scan_code_table[input]) {
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
        } else if (flag[BACKSPACE_PRESSED]) {
            delete_last();
            key_buf_cnt = max(key_buf_cnt-1, 0);
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
    for(i = 0; i < KEYBOARD_BUF_SIZE; i++){
        flag[i] = 0;
        keyboard_buf[i] = 0;
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

    // loop until exit
    while(!end_flag){
        // prevent interrupt from modifying the line buffer
        cli();
        // detect enter
        for(i = 0; i <= nbytes && (i <= key_buf_cnt - 1); i++){
            if(keyboard_buf[i] == '\n'){
                j = i;
                end_flag = 1;
                break;
            }
        }

        // no enter but nbytes have been written
        if(end_flag != 1 && (i >= nbytes - 1)) {
            j = nbytes;
            end_flag = 1;
        }

        sti();
    }

    // new critical section
    cli();

    // copy to user buf
    memcpy(buf,keyboard_buf,j);

    // set delete_length to handle two conditions of \n and no \n
    delete_length = j + (keyboard_buf[j] == '\n');

    for( i = delete_length; i < KEYBOARD_BUF_SIZE; i++){
        keyboard_buf[i - delete_length] = keyboard_buf[i];
    }

    // update key_buf_cnt
    key_buf_cnt -= j;
    if(key_buf_cnt < 0) key_buf_cnt = 0;

    sti();

    return 0;

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









