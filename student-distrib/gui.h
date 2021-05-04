#ifndef _GUI_H
#define _GUI_H

#include "vga.h"

#define COPY_NUM 4096


void init_gui();

void render_window(int x, int y, int width, int height, char* title, uint8_t is_focus);

void render_font(int x_start, int y_start, char ch, uint32_t color);

void render_string(int x_start, int y_start, char* string, uint32_t color);

extern int month,day,sec,mins,hour;
void setup_status_bar();

void render_mouse(int x, int y);

void erase_mouse(int x, int y);

void copy_vedio_mem(void* dest);

uint32_t mouse_click_check(int32_t x, int32_t y);

#endif
