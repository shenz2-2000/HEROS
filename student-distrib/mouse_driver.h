//
// Created by BOURNE on 2021/5/1.
//

#ifndef MP3_GROUP_14_MOUSE_DRIVER_H
#define MP3_GROUP_14_MOUSE_DRIVER_H

//#define Y_overflow (1<<7)
//#define X_overflow (1<<6)
//#define MID_PRESSED (1<<2)
//#define RIGHT_PRESSED (1<<1)
//#define LEFT_PRESSED (1)
//#define TEXT_MODE_WIDTH    80
//#define TEXT_MODE_HEIGHT    25
//#define X_SIGN     (1<<4)
//#define Y_SIGN     (1<<5)
//#define PRECEDE_COMMAND   0xD4
//#define MOUSE_DATA_PORT   0x60
//#define MOUSE_CHECK_PORT  0x64
//#define MOUSE_ACKNOWLEDGE 0xFA
//#define MOUSE_RESET       0xFF


#define MOUSE_IRQ_NUM 12
extern void mouse_init();
extern void mouse_interrupt_handler();
extern int need_redraw_background;
extern int need_change_focus;
//--------------------------
//#define MOUSE_PORT_60 0x60
//#define PORT_64 0x64
//#define ACK 0xFA
#define MOUSE_DATA_PORT 0x60
#define MOUSE_CHECK_PORT 0x64
#define MOUSE_ACKNOWLEDGE 0xFA
#define RESET 0xFF
#define Y_OVERFLOW 0x080
#define X_OVERFLOW 0x40
#define Y_SIGN 0x20
#define X_SIGN 0x10
#define VERIFY_ONE 0x08
#define MID_BUTTON 0x04
#define RIGHT_BUTTON 0x02
#define LEFT_BUTTON 0x01
#define RESET 0xFF
#define PRE_COMMAND 0xD4
//#define VGA_WIDTH 1024
//#define VGA_HEIGHT 768
//#define BLACK 0xFF000000
//#define WHITE 0xFFFFFFFF
extern void set_mouse_cursor(int x, int y);
extern void mouse_init();
extern void erase_mouse();
extern void render_mouse(int,int);
int check_mouse_in_button(int mouse_x,int mouse_y);

extern int16_t mouse_x;
extern int16_t mouse_y;
extern int16_t prev_mouse_x;
extern int16_t prev_mouse_y;
extern int prev_draw_x;
extern int prev_draw_y;


// several global indicators
extern uint32_t left_pressed;
extern uint32_t right_pressed;
extern uint32_t middle_pressed;

#endif //MP3_GROUP_14_MOUSE_DRIVER_H
