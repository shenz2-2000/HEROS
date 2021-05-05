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
#define C_PRESSED     0x2E
#define KEYBOARD_BUF_SIZE 128
#define F1_PRESSED 0x3B
#define F2_PRESSED 0x3C
#define F3_PRESSED 0x3D
#define ALT_PRESSED 0x38
#define MAX_TERMINAL 3
// function declarations

// vidmap usage
#define VM_INDEX      0xB8000
#define VM_PTE        0xB8

#define U_VM_PDE 33

#define BITS_4K        4096     // 0x1000
#define BITS_4M        4194304  // 0x40000

// terminal status
#define TERMINAL_ON     MAX_TERMINAL+2  // avoid interference
#define TERMINAL_OFF    MAX_TERMINAL+1

void terminal_initialization();
void print_terminal_info();

int32_t terminal_open(const uint8_t* filename);
int32_t terminal_close(int32_t fd);
int32_t terminal_write(int32_t fd, const void* buf, int32_t nbytes);
int32_t terminal_read(int32_t fd, void* buf, int32_t nbytes);
typedef struct terminal_struct_t terminal_struct_t;
struct terminal_struct_t{
    int valid, id, buf_cnt, user_ask, screen_x, screen_y;
    char buf[KEYBOARD_BUF_SIZE];
};

//extern terminal_struct_t null_terminal;

void terminal_init();
terminal_struct_t* terminal_allocate();
void terminal_deallocate(terminal_struct_t* terminal);
void terminal_set_running(terminal_struct_t *terminal);
//int terminal_turn_on(terminal_struct_t *terminal);
int switch_terminal(terminal_struct_t *old_terminal, terminal_struct_t *new_terminal);
int terminal_vidmap(terminal_struct_t *terminal);
int terminal_vidmap_SVGA(terminal_struct_t *terminal);
terminal_struct_t* get_showing_terminal();
// vidmap
void vidmap_init();
void set_video_memory(terminal_struct_t *terminal);
void set_video_memory_SVGA(terminal_struct_t *terminal);
void clear_video_memory();

#endif
