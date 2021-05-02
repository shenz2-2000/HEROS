//
// Created by BOURNE on 2021/5/1.
//

#include "lib.h"
#include "mouse_driver.h"
#include "link.h"
#include "process.h"
#include "idt.h"
#include "i8259.h"
int32_t left_button_pressed = 0;
uint8_t prev_input;
uint32_t mouse_x= 10, mouse_y= 10;

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
void write_byte_to_port(uint8_t byte, uint8_t port) {
    while(1) {
        // All output to port 0x60 or 0x64 must be preceded by waiting for bit 1 (value=2) of port 0x64 to become clear.
        if (0 == (inb(PORT_64) & 0x2)) {
            break;
        }
    }
    outb(byte, port);
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

void mouse_init() {
    send_command_to_60(RESET);
//    // Set Compaq Status/Enable IRQ12
//    // Send the command byte 0x20 ("Get Compaq Status Byte") to the PS2 controller on port 0x64
    write_byte_to_port(0x20, PORT_64);
//    // Very next byte returned should be the Status byte
    uint8_t compaq_status = read_from_60();
    compaq_status |= 0x2;
    compaq_status &= ~(0x20);
    write_byte_to_port(0x60, PORT_64);
    write_byte_to_port(compaq_status, MOUSE_PORT_60);
//    // Enable Packet Streaming
    send_command_to_60(0xF4);

}
ASMLINKAGE void mouse_handler(hw_context hw) {

    send_eoi(hw.irq);
//    if (!(inb(MOUSE_CHECK_PORT)&1) || !(inb(MOUSE_CHECK_PORT)&0x20)) return;
//    uint8_t input = inb(MOUSE_DATA_PORT);
//    if (input == MOUSE_ACKNOWLEDGE) return;// ACK signal
//    int32_t x_delta = port_read(MOUSE_DATA_PORT,MOUSE_CHECK_PORT),y_delta = port_read(MOUSE_DATA_PORT,MOUSE_CHECK_PORT);
//    if (!(input & 0x08)) return;
//    if (input&Y_overflow || input&X_overflow) return;
//    if (!(prev_input&LEFT_PRESSED) && (input&LEFT_PRESSED)) {
//        left_button_pressed=1;
//        printf("LEFT_PRESS\n");
//    }
//    if ((prev_input&LEFT_PRESSED) && !(input&LEFT_PRESSED)) {
//        left_button_pressed=0;
//        printf("LEFT_RELEASE\n");
//    }
//    if (input & MID_PRESSED) printf("MID_PRESS\n");
//    if (input & RIGHT_PRESSED) printf("RIHGT_PRESS\n");
//    if (input&X_SIGN) x_delta|=0xFFFFFF00;
//    if (input&Y_SIGN) y_delta|=0xFFFFFF00;
//    y_delta*=-1;
//    mouse_x+=x_delta;
//    mouse_x=max(mouse_x,0), mouse_x = min(mouse_x, TEXT_MODE_HEIGHT);
//    mouse_y+=y_delta;
//    mouse_y=max(mouse_y,0), mouse_y = min(mouse_y, TEXT_MODE_WIDTH);
//    prev_input = input;
    //set_blue_cursor(mouse_x, mouse_y);
}

