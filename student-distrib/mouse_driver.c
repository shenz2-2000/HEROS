//
// Created by BOURNE on 2021/5/1.
//

#include "lib.h"
#include "mouse_driver.h"

void set_mouse_cursor(int x, int y){
    x = (x << 5) | 0x0010;
    outw(x,0x3c4);

    y = (y << 5) | 0x0011;
    outw(y,0x3c4);
}


