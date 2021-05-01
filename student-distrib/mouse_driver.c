//
// Created by BOURNE on 2021/5/1.
//

#include "lib.h"
#include "mouse_driver.h"
#include "link.h"
#include "process.h"
#include "idt.h"
#define Y_overflow (1<<7)
#define X_overflow (1<<6)
#define MID_PRESSED (1<<2)
#define RIGHT_PRESSED (1<<1)
#define LEFT_PRESSED (1)
#define TEXT_MODE_WIDTH    80
#define TEXT_MODE_HEIGHT    25
#define X_SIGN     (1<<4)
#define Y_SIGN     (1<<5)
int32_t left_button_pressed = 0;
uint8_t prev_input;
uint32_t mouse_x=945, mouse_y=30;
//void set_mouse_cursor(int x, int y){
//    x = (x << 5) | 0x0010;
//    outw(x,0x3c4);
//
//    y = (y << 5) | 0x0011;
//    outw(y,0x3c4);
//}

ASMLINKAGE void mouse_handler(hw_context hw) {
    uint8_t input = inb(0x60);
    if (input == 0xFA) return;// ACK signal
    int32_t x_delta = port_read(),y_delta = port_read();
    if (input&Y_overflow || input&X_overflow) return;
    if (!(prev_input&LEFT_PRESSED) && (input&LEFT_PRESSED)) {
        left_button_pressed=1;
        printf("LEFT_PRESS\n");
    }
    if ((prev_input&LEFT_PRESSED) && !(input&LEFT_PRESSED)) {
        left_button_pressed=0;
        printf("LEFT_RELEASE\n");
    }
    if (input & MID_PRESSED) printf("MID_PRESS\n");
    if (input & RIGHT_PRESSED) printf("RIHGT_PRESS\n");
    if (input&X_SIGN) x_delta|=0xFFFFFF00;
    if (input&Y_SIGN) y_delta|=0xFFFFFF00;
    y_delta*=-1;
    mouse_x+=x_delta;
    mouse_x=max(mouse_x,0), mouse_x = min(mouse_x, TEXT_MODE_HEIGHT);
    mouse_y+=y_delta;
    mouse_y=max(mouse_y,0), mouse_y = min(mouse_y, TEXT_MODE_WIDTH);
    prev_input = input;
    set_blue_cursor(mouse_x, mouse_y);
}
