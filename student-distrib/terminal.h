#ifndef _TEM_H
#define _TEM_H

// magic number
#define KEY_BOARD_PRESSED 0x60
#define RELEASE_DIFF      0x80
#define CTRL_PRESSED      0x1D
#define CAPLCL_PRESSED      0x3A
#define LEFT_SHIFT_PRESSED  0x2A
#define RIGHT_SHIFT_PRESSED 0x36
#define L_PRESSED           0x26
#define BACKSPACE_PRESSED   0x0E
#define KEYBOARD_PORT 0x60
#define KEYBOARD_BUF_SIZE 128

// function declarations
void keyboard_interrupt_handler();
void terminal_initialization();

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);

#endif
