//
// Created by BOURNE on 2021/5/1.
//

#ifndef MP3_GROUP_14_MOUSE_DRIVER_H
#define MP3_GROUP_14_MOUSE_DRIVER_H

#define Y_overflow (1<<7)
#define X_overflow (1<<6)
#define MID_PRESSED (1<<2)
#define RIGHT_PRESSED (1<<1)
#define LEFT_PRESSED (1)
#define TEXT_MODE_WIDTH    80
#define TEXT_MODE_HEIGHT    25
#define X_SIGN     (1<<4)
#define Y_SIGN     (1<<5)
#define PRECEDE_COMMAND   0xD4
#define MOUSE_DATA_PORT   0x60
#define MOUSE_CHECK_PORT  0x64
#define MOUSE_ACKNOWLEDGE 0xFA
#define MOUSE_RESET       0xFF

extern void set_mouse_cursor(int x, int y);


#endif //MP3_GROUP_14_MOUSE_DRIVER_H
