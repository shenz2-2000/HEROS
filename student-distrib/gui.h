#ifndef _GUI_H
#define _GUI_H

#include "vga.h"

void init_gui();

void render_window(int x, int y, int width, int height, char* title, uint8_t is_focus);

void render_font(int x_start, int y_start, char ch, uint32_t color);

void render_string(int x_start, int y_start, char* string, uint32_t color);

void render_mouse(int x, int y);

void erase_mouse(int x, int y);

#endif
