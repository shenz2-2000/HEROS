//
// Created by BOURNE on 2021/5/1.
//

#include "lib.h"
#include "mouse_driver.h"
#include "link.h"
#include "process.h"
#include "idt.h"
#include "i8259.h"




#define MOUSE_PORT_60 0x60
#define PORT_64 0x64
#define ACK 0xFA
#define RESET 0xFF
/**
 * The first byte of the packet received from mouse is in the following format
 * Y overflow	X overflow	Y sign bit	X sign bit	Always 1	Middle Btn	Right Btn	Left Btn
 * Bit 7            6           5          4           3            2           1          0
 */
#define Y_OVERFLOW 0x080
#define X_OVERFLOW 0x40
#define Y_SIGN 0x20
#define X_SIGN 0x10
#define VERIFY_ONE 0x08
#define MID_BUTTON 0x04
#define RIGHT_BUTTON 0x02
#define LEFT_BUTTON 0x01

// Local helper functions
uint8_t read_from_60();
void write_byte_to_port(uint8_t byte, uint8_t port);
void send_command_to_60(uint8_t command);
static int16_t mouse_x = 0;
static int16_t mouse_y = 0;



/**
 *
 * @param command - the command we need to send to the mouse
 */
void send_command_to_60(uint8_t command){
    // Sending a command or data byte to the mouse (to port 0x60) must be preceded by sending a 0xD4 byte to port 0x64.
    write_byte_to_port(0xD4, PORT_64);
    write_byte_to_port(command, MOUSE_PORT_60);
    // Wait for ACK
    while (1) {
        if (ACK == read_from_60()) {
            break;
        }
    }
}
uint8_t read_from_60() {
    // Bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
    while(1){
        if (1 == (inb(PORT_64) & 0x1)) {
            break;
        }
    }
    return inb(MOUSE_PORT_60);
}
void write_byte_to_port(uint8_t byte, uint8_t port) {
    while(1) {
        // All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
        if (0 == (inb(PORT_64) & 0x2)) {
            break;
        }
    }
    outb(byte, port);
}

void mouse_init() {
    send_command_to_60(RESET);
    // Set Compaq Status/Enable IRQ12
    // Send the command byte 0x20 ("Get Compaq Status Byte") to the PS2 controller on port 0x64
    write_byte_to_port(0x20, PORT_64);
    // Very next byte returned should be the Status byte
    uint8_t compaq_status = read_from_60();
    compaq_status |= 0x2;
    compaq_status &= ~(0x20);
    write_byte_to_port(0x60, PORT_64);
    write_byte_to_port(compaq_status, MOUSE_PORT_60);
    // Aux Input Enable Command
//    write_byte_to_port(0xA8, PORT_64);
//    while (1) {
//        if (ACK == read_from_60()) {
//            break;
//        }
//    }
    // Enable Packet Streaming
    send_command_to_60(0xF4);
}
void mouse_interrupt_handler() {
    /**
     * Firstly, we need to check two things:
     * 1. Whether port 60 is ready for read (check bit 1 of port 64)
     * 2. Whether port 60 gives data for keyboard or mouse (check bit 5 of port 64)
     */
    if (0 == (inb(PORT_64) & 0x1)) {
        return;
    }
    if (0 == inb(PORT_64) & 0x20) {
        return;
    }
    uint8_t flags = inb(MOUSE_PORT_60);
    if (ACK == flags) {
        return;  // ignore the ACK
    } else {
        int32_t x_movement = read_from_60();
        int32_t y_movement = read_from_60();
        if (!(flags & VERIFY_ONE)) {
            printf("mouse packet not aligned!\n");
            return;
        }
        if ((flags & X_OVERFLOW) || (flags & Y_OVERFLOW)) {
            printf("mouse overflow occurred!\n");
            return;
        }
        if (flags & LEFT_BUTTON) {
            printf("left button pressed\n");
        }
        if (flags & MID_BUTTON) {
            printf("middle button pressed\n");
        }
        if (flags & RIGHT_BUTTON) {
            printf("right button pressed\n");
        }

        if (flags & X_SIGN) {
            x_movement |= 0xFFFFFF00;
        }
        if (mouse_x + x_movement < 0) {
            mouse_x = 0;
        } else if (mouse_x + x_movement > 80 - 1) {
            mouse_x = 80 - 1;
        } else {
            mouse_x += x_movement;
        }
        printf("X movement = %d mouse_x = %d\n", x_movement, mouse_x);
        if (flags & Y_SIGN) {
            y_movement |= 0xFFFFFF00;
        }
        // TODO: For y_movement, should negate the result, don't know why.
        if (mouse_y - y_movement < 0) {
            mouse_y = 0;
        } else if (mouse_y - y_movement > 200 - 1) {
            mouse_y = 200 - 1;
        } else {
            mouse_y -= y_movement;
        }
        printf("Y movement is %d mouse_y = %d\n", y_movement, mouse_y);
    }
}


//------------------------------------------------------------------------------------------------------------------------------------------------
//int32_t left_button_pressed = 0;
//uint8_t prev_input;
////uint32_t mouse_x= 10, mouse_y= 10;
//
//
//// Local constants
//#define MOUSE_PORT_60 0x60
//#define PORT_64 0x64
//#define ACK 0xFA
//#define RESET 0xFF
//#define VGA_WIDTH 1024
//#define VGA_HEIGHT 768
//#define BLACK 0xFF000000
//#define WHITE 0xFFFFFFFF
///**
// * The first byte of the packet received from mouse is in the following format
// * Y overflow	X overflow	Y sign bit	X sign bit	Always 1	Middle Btn	Right Btn	Left Btn
// * Bit 7            6           5          4           3            2           1          0
// */
//#define Y_OVERFLOW 0x080
//#define X_OVERFLOW 0x40
//#define Y_SIGN 0x20
//#define X_SIGN 0x10
//#define VERIFY_ONE 0x08
//#define MID_BUTTON 0x04
//#define RIGHT_BUTTON 0x02
//#define LEFT_BUTTON 0x01
//#define CURSOR_WIDTH 8
//#define CURSOR_HEIGHT 8
//
//// Local helper functions
//uint8_t read_from_60();
//void write_byte_to_port(uint8_t byte, uint8_t port);
//void send_command_to_60(uint8_t command);
//static int16_t mouse_x = 10;
//static int16_t mouse_y = 10;
//
//static uint8_t last_flags = 0;
//
//
//void set_mouse_cursor(int x, int y){
//    x = (x << 5) | 0x0010;
//    outw(x,0x3c4);
//
//    y = (y << 5) | 0x0011;
//    outw(y,0x3c4);
//
//}
//
///**
// *
// * @param command - the command we need to send to the mouse
// */
//void send_command_to_60(uint8_t command){
//    int i = 100000;
//    // Sending a command or data byte to the mouse (to port 0x60) must be preceded by sending a 0xD4 byte to port 0x64.
//    write_byte_to_port(0xD4, PORT_64);
//    write_byte_to_port(command, MOUSE_PORT_60);
//    // Wait for ACK
//    while (i >= 0) {
//        if (ACK == read_from_60()) {
//            break;
//        }
//        i--;
//    }
//}
//uint8_t read_from_60() {
//    // Bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
//    while(1){
//        if (1 == (inb(PORT_64) & 0x1)) {
//            break;
//        }
//    }
//    return inb(MOUSE_PORT_60);
//}
//void write_byte_to_port(uint8_t byte, uint8_t port) {
//    while(1) {
//        // All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
//        if (0 == (inb(PORT_64) & 0x2)) {
//            break;
//        }
//    }
//    outb(byte, port);
//}
//
//void mouse_init() {
//    mouse_in_use = 1;
//    send_command_to_60(RESET);
//    // Set Compaq Status/Enable IRQ12
//    // Send the command byte 0x20 ("Get Compaq Status Byte") to the PS2 controller on port 0x64
//    write_byte_to_port(0x20, PORT_64);
//    // Very next byte returned should be the Status byte
//    uint8_t compaq_status = read_from_60();
//    compaq_status |= 0x2;
//    compaq_status &= ~(0x20);
//    write_byte_to_port(0x60, PORT_64);
//    write_byte_to_port(compaq_status, MOUSE_PORT_60);
//
//    // Enable Packet Streaming
//    send_command_to_60(0xF4);
//    // TODO
//    set_mouse_cursor(mouse_x, mouse_y);
//    mouse_in_use = 0;
//}
//ASMLINKAGE void mouse_interrupt_handler(hw_context hw_context) {
//    mouse_in_use = 1;
//
//    send_eoi(hw_context.irq);
//
//    /**
//     * Firstly, we need to check two things:
//     * 1. Whether port 60 is ready for read (check bit 1 of port 64)
//     * 2. Whether port 60 gives data for keyboard or mouse (check bit 5 of port 64)
//     */
//    if (0 == (inb(PORT_64) & 0x1)) {
//        return;
//    }
//    if (0 == (inb(PORT_64) & 0x20)) {
//        return;
//    }
//    uint8_t flags = inb(MOUSE_PORT_60);
//    if (ACK == flags) {
//        return;  // ignore the ACK
//    } else {
//        int32_t x_movement = read_from_60();
//        int32_t y_movement = read_from_60();
//        if (!(flags & VERIFY_ONE)) {
////            printf("mouse packet not aligned!\n");
//            return;
//        }
//        if ((flags & X_OVERFLOW) || (flags & Y_OVERFLOW)) {
////            printf("mouse overflow occurred!\n");
//            return;
//        }
//        // Button press
//        if (!(last_flags & LEFT_BUTTON) && (flags & LEFT_BUTTON)) {
//              printf("left button pressed\n");
////            gui_handle_mouse_press(mouse_x, mouse_y);
//        }
//        if ((last_flags & LEFT_BUTTON) && !(flags & LEFT_BUTTON)) {
//              printf("left button released\n");
////            gui_handle_mouse_release(mouse_x, mouse_y);
//        }
//        if (flags & MID_BUTTON) {
//              printf("middle button pressed\n");
//        }
//        if (flags & RIGHT_BUTTON) {
//              printf("right button pressed\n");
//        }
//        // Preserve the original color of pixels at old position of cursor
////        int i, j;
////        for (i = 0; i < CURSOR_WIDTH; i++) {
////            for (j = 0; j < CURSOR_HEIGHT; j++) {
////                vga_set_color_argb(origin_pixels[i][j] | 0xFF000000);
////                vga_draw_pixel(mouse_x + i, mouse_y + j);
////            }
////        }
//        // Update mouse location
//        if (flags & X_SIGN) {
//            x_movement |= 0xFFFFFF00;
//        }
//        //printf("X movement = %d mouse_x = %d\n", x_movement, mouse_x);
//        if (flags & Y_SIGN) {
//            y_movement |= 0xFFFFFF00;
//        }
//        //printf("Y movement = %d mouse_y = %d\n", y_movement, mouse_y);
//
//        // TODO: For y_movement, should negate the result, don't know why.
//        y_movement = -y_movement;
//
////        if (gui_handle_mouse_move(x_movement, y_movement) != 0) {  // movement that can drag window out of screen
////            x_movement = y_movement = 0;  // cancel the movement
////        }
//
//        if (mouse_x + x_movement < 0) {
//            mouse_x = 0;
//        } else if (mouse_x + x_movement > VGA_WIDTH - 1 - CURSOR_WIDTH) {
//            mouse_x = VGA_WIDTH - 1- CURSOR_WIDTH;
//        } else {
//            mouse_x += x_movement;
//        }
//
//        if (mouse_y + y_movement < 0) {
//            mouse_y = 0;
//        } else if (mouse_y + y_movement > VGA_HEIGHT - 1 - CURSOR_HEIGHT) {
//            mouse_y = VGA_HEIGHT - 1 - CURSOR_HEIGHT;
//        } else {
//            mouse_y += y_movement;
//        }
//        set_mouse_cursor(mouse_x, mouse_y);
//
//
//
//        // Record the current pixels covered by the cursor
////        for (i = 0; i < CURSOR_WIDTH; i++) {
////            for (j = 0; j < CURSOR_HEIGHT; j++) {
////                origin_pixels[i][j] = vga_get_pixel(mouse_x + i, mouse_y + j);
////            }
////        }
////
////        vga_set_color_argb(WHITE);
////        for (i = 0; i < CURSOR_WIDTH; i++) {
////            for (j = 0; j < CURSOR_HEIGHT; j++) {
////                vga_draw_pixel(mouse_x + i, mouse_y + j);
////            }
////        }
//
//        last_flags = flags;
//        mouse_in_use = 0;
//    }
//}





//void set_mouse_cursor(int x, int y){
//    x = (x << 5) | 0x0010;
//    outw(x,0x3c4);
//
//    y = (y << 5) | 0x0011;
//    outw(y,0x3c4);
//}
//
//
//
//// for mouse, just port_read(MOUSE_DATA_PORT, MOUSE_CHECK_PORT)
//uint8_t port_read(uint8_t read_port, uint8_t check_port){
//
//    // we need to check the first bit in check_port
//    while(1){
//        if( ((inb(check_port)) & 1)  == 1) break;
//    }
//
//    return inb(read_port);
//
//}
//
//// for mouse, just port_write(content, MOUSE_DATA_PORT, MOUSE_CHECK_PORT)
//void port_write(uint8_t content, uint8_t target_port, uint8_t check_port){
//
//    // we need to check the device is not keyboard
//    while(1){
//        if( (( inb(check_port) & 0x2 ) == 0)) break;
//    }
//
//    outb(content, target_port);
//
//}
//
//void send_command_to_mouse(uint8_t command){
//
//    // two steps to send command
//    port_write(PRECEDE_COMMAND,MOUSE_CHECK_PORT,MOUSE_CHECK_PORT);
//    port_write(command,MOUSE_DATA_PORT,MOUSE_CHECK_PORT);
//
//    // wait for success
//    while(1){
//        if (port_read(MOUSE_DATA_PORT,MOUSE_CHECK_PORT) == MOUSE_ACKNOWLEDGE) break;
//    }
//}
//
//void initialize_mouse(){
//    send_command_to_mouse(MOUSE_RESET);
//    port_write(0x20,MOUSE_CHECK_PORT,MOUSE_CHECK_PORT);
//
//    uint8_t cur_status = port_read(MOUSE_DATA_PORT,MOUSE_CHECK_PORT);
//    cur_status = (cur_status | 0x2) & ( ~(0x20) );
//
//    port_write(0x60,MOUSE_CHECK_PORT,MOUSE_CHECK_PORT);
//    port_write(cur_status,MOUSE_DATA_PORT,MOUSE_CHECK_PORT);
//
//    send_command_to_mouse(0xF4);
//
//    //set_mouse_cursor(mouse_x, mouse_y);
//
//}
//void write_byte_to_port(uint8_t byte, uint8_t port) {
//    while(1) {
//        // All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
//        if (0 == (inb(PORT_64) & 0x2)) {
//            break;
//        }
//    }
//    outb(byte, port);
//}
//uint8_t read_from_60() {
//    // Bytes cannot be read from port 0x60 until bit 0 (value=1) of port 0x64 is set.
//    while(1){
//        if (1 == (inb(PORT_64) & 0x1)) {
//            break;
//        }
//    }
//    return inb(MOUSE_PORT_60);
//}
//
//void send_command_to_60(uint8_t command){
//    // Sending a command or data byte to the mouse (to port 0x60) must be preceded by sending a 0xD4 byte to port 0x64.
//    write_byte_to_port(0xD4, PORT_64);
//    write_byte_to_port(command, MOUSE_PORT_60);
//    // Wait for ACK
//    while (1) {
//        if (ACK == read_from_60()) {
//            break;
//        }
//    }
//}
//
//void mouse_init() {
//    send_command_to_60(RESET);
////    // Set Compaq Status/Enable IRQ12
////    // Send the command byte 0x20 ("Get Compaq Status Byte") to the PS2 controller on port 0x64
//    write_byte_to_port(0x20, PORT_64);
////    // Very next byte returned should be the Status byte
//    uint8_t compaq_status = read_from_60();
//    compaq_status |= 0x2;
//    compaq_status &= ~(0x20);
//    write_byte_to_port(0x60, PORT_64);
//    write_byte_to_port(compaq_status, MOUSE_PORT_60);
////    // Enable Packet Streaming
//    send_command_to_60(0xF4);
//
//}
//ASMLINKAGE void mouse_handler(hw_context hw) {
//
//    send_eoi(hw.irq);
////    if (!(inb(MOUSE_CHECK_PORT)&1) || !(inb(MOUSE_CHECK_PORT)&0x20)) return;
////    uint8_t input = inb(MOUSE_DATA_PORT);
////    if (input == MOUSE_ACKNOWLEDGE) return;// ACK signal
////    int32_t x_delta = port_read(MOUSE_DATA_PORT,MOUSE_CHECK_PORT),y_delta = port_read(MOUSE_DATA_PORT,MOUSE_CHECK_PORT);
////    if (!(input & 0x08)) return;
////    if (input&Y_overflow || input&X_overflow) return;
////    if (!(prev_input&LEFT_PRESSED) && (input&LEFT_PRESSED)) {
////        left_button_pressed=1;
////        printf("LEFT_PRESS\n");
////    }
////    if ((prev_input&LEFT_PRESSED) && !(input&LEFT_PRESSED)) {
////        left_button_pressed=0;
////        printf("LEFT_RELEASE\n");
////    }
////    if (input & MID_PRESSED) printf("MID_PRESS\n");
////    if (input & RIGHT_PRESSED) printf("RIHGT_PRESS\n");
////    if (input&X_SIGN) x_delta|=0xFFFFFF00;
////    if (input&Y_SIGN) y_delta|=0xFFFFFF00;
////    y_delta*=-1;
////    mouse_x+=x_delta;
////    mouse_x=max(mouse_x,0), mouse_x = min(mouse_x, TEXT_MODE_HEIGHT);
////    mouse_y+=y_delta;
////    mouse_y=max(mouse_y,0), mouse_y = min(mouse_y, TEXT_MODE_WIDTH);
////    prev_input = input;
//    //set_blue_cursor(mouse_x, mouse_y);
//}

