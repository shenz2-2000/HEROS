//
// Created by BOURNE on 2021/5/1.
//

#include "lib.h"
#include "mouse_driver.h"
#include "gui.h"
#include "vga.h"






// Local helper functions
uint8_t mouse_port_read();
void port_write(uint8_t byte, uint8_t port);
void command_mouse(uint8_t command);
int16_t mouse_x = 0;
int16_t mouse_y = 0;
int16_t prev_mouse_x = 0;
int16_t prev_mouse_y = 0;

// several global indicators
uint32_t left_pressed = 0;
uint32_t right_pressed = 0;
uint32_t middle_pressed = 0;

static uint8_t prev_flag = 0;


// set sample rate
void set_sample_rate(uint8_t sample_rate){

    uint8_t ack;

    outb(0xD4,MOUSE_CHECK_PORT);
    outb(0xF3,MOUSE_DATA_PORT);

    // wait until we can read
    while( !(inb(MOUSE_CHECK_PORT) & 1));

    ack = inb(MOUSE_DATA_PORT);
    outb(0xD4,MOUSE_CHECK_PORT);
    outb(sample_rate,MOUSE_DATA_PORT);

    // wait until we can read
    while( !(inb(MOUSE_CHECK_PORT) & 1));

    ack = inb(MOUSE_DATA_PORT);

}



void set_mouse_cursor(int x, int y) {
    x = (x << 5) | 0x0010;
    outw(x, 0x3c4);

    y = (y << 5) | 0x0011;
    outw(y, 0x3c4);
}


/**
 *
 * @param command - the command we need to send to the mouse
 */
void command_mouse(uint8_t command){
    // Sending a command or data byte to the mouse (to port 0x60) must be preceded by sending a 0xD4 byte to port 0x64.
    port_write(PRE_COMMAND, MOUSE_CHECK_PORT);
    port_write(command, MOUSE_DATA_PORT);
    // Wait for MOUSE_ACKNOWLEDGE
    while (1) {
        if (MOUSE_ACKNOWLEDGE == mouse_port_read()) {
            break;
        }
    }
}

uint8_t mouse_port_read() {
    // Bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
    while(1){
        if (1 == (inb(MOUSE_CHECK_PORT) & 0x1)) {
            break;
        }
    }
    return inb(MOUSE_DATA_PORT);
}

void port_write(uint8_t byte, uint8_t port) {
    while(1) {
        // All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
        if (0 == (inb(MOUSE_CHECK_PORT) & 0x2)) {
            break;
        }
    }
    outb(byte, port);
}

void mouse_init() {
    command_mouse(RESET);

    // Send the command 0x20
    port_write(0x20, MOUSE_CHECK_PORT);

    // Very next byte returned should be the Status byte
    uint8_t status = mouse_port_read();
    status =(status |0x2) & (~(0x20));

    // write to two ports
    port_write(0x60, MOUSE_CHECK_PORT);
    port_write(status, MOUSE_DATA_PORT);

    // set blue cursor on screen
    // TODO: delete this after enabling VGA
    command_mouse(0xF4);
    set_blue_cursor(mouse_x, mouse_y);

    // set sample rate to slow down mouse
//    set_sample_rate(10);

    left_pressed = 0;
    right_pressed = 0;
    middle_pressed = 0;
}

void mouse_interrupt_handler() {

    // avoid collide with keyboard
    if (0 == (inb(MOUSE_CHECK_PORT) & 0x1)) {
        return;
    }
    if (0 == (inb(MOUSE_CHECK_PORT) & 0x20)) {
        return;
    }
    uint8_t flags = inb(MOUSE_DATA_PORT);

    if (MOUSE_ACKNOWLEDGE == flags) {
        return;  // ignore the MOUSE_ACKNOWLEDGE
    } else {
        int32_t d = mouse_port_read();
        int32_t x_movement;
        int32_t y_movement;

        if (!(flags & VERIFY_ONE)) {
            //printf("mouse  not aligned!\n");
            return;
        }
        if ((flags & X_OVERFLOW) || (flags & Y_OVERFLOW)) {
            //printf("mouse overflow!\n");
            return;
        }
        if ((flags & LEFT_BUTTON) ) {
           // printf("left button pressed\n");
           left_pressed = 1;
           //render_string(512,384,"left click is ok!",0xDC143C);
        }
        else if(!(flags & LEFT_BUTTON) ){
            left_pressed = 0;
            //Rdraw(100,8,512,384,0XFFFFFF);
        }

        if ((flags & MID_BUTTON)) {
            // printf("left button pressed\n");
            middle_pressed = 1;

        }
        if(!(flags & MID_BUTTON) && (prev_flag & MID_BUTTON)){
            middle_pressed = 0;
        }

        if ((flags & RIGHT_BUTTON) ) {
            // printf("left button pressed\n");
            right_pressed = 1;
            //render_string(512,484,"right click is ok!",0xDC143C);
        }
        else if(!(flags & RIGHT_BUTTON)){
            right_pressed = 0;
            //Rdraw(100,8,512,484,0XFFFFFF);
        }

        // value manipulation
        x_movement = d - ((flags << 4) & 0x100);
        d = mouse_port_read();
        y_movement = d - ((flags << 3) & 0x100);

        // slow down a little
        y_movement = y_movement  ;
        x_movement = x_movement  ;

        // update x y
        if (mouse_x + x_movement < 0) {
            mouse_x = 0;
        } else if (mouse_x + x_movement > VGA_DIMX-1) {
            mouse_x = VGA_DIMX - 1;
        } else {
            mouse_x += x_movement;
        }


        if (mouse_y - y_movement < 0) {
            mouse_y = 0;
        } else if (mouse_y - y_movement > VGA_DIMY -1 ) {
            mouse_y = VGA_DIMY - 1;
        } else {
            mouse_y -= y_movement;
        }

        if(mouse_x == prev_mouse_x && mouse_y == prev_mouse_y){
            return;
        }
        else{
            erase_mouse();
            render_mouse(mouse_x,mouse_y);
            prev_mouse_x = mouse_x;
            prev_mouse_y = mouse_y;
        }

//        if(512 >= mouse_x && 512 <= (mouse_x + 15) && 384 >= mouse_y && 384 <= (mouse_y + 7) && left_pressed){
//            render_string(512,384,"left click is ok!",0xDC143C);
//        }



//        if(left_pressed){
//            render_string(512,384,"left click is ok!",0xDC143C);
//        }
//        else{
//            Rdraw(100,8,512,384,0XFFFFFF);
//        }
//
//        if(right_pressed){
//            render_string(512,484,"right click is ok!",0xDC143C);
//        }
//        else{
//            Rdraw(100,8,512,484,0XFFFFFF);
//        }

        prev_flag = flags;

    }
}


