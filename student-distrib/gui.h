#ifndef _GUI_H
#define _GUI_H

#include "vga.h"

#define COPY_NUM 4096
#define MODEX_TER_COLS 80  // Same as the original one
#define MODEX_TER_ROWS 25
#define STATUS_BAR_HEIGHT 31

typedef struct window_t {
    int32_t pos_x, pos_y, width, height;
    int id;
    int priority,active;
} window_t;

void init_gui();

void render_window(int x, int y, int width, int height, char* title, int focus);

void render_word(int x_start, int y_start, char ch, uint32_t color);

void render_sentence(int x_start, int y_start, char* string, uint32_t color);

extern int month,day,sec,mins,hour;

void setup_status_bar();

void render_mouse(int x, int y);

void erase_mouse();

void copy_vedio_mem(uint8_t * dest);

uint32_t mouse_click_check(int32_t x, int32_t y);

int32_t check_in_status_bar();

void restore_background(int x,int y);

void restore_status_bar();

int32_t check_in_background();

void draw_terminal(char* video_cache,int terminal_id, int focus);

void update_priority(int terminal_id);

int check_mouse_in_which_terminal(int32_t x, int32_t y);

int32_t check_in_window(int,int);

void render_music_icon(int x, int y);

void render_terminal_button();

int render_randomness();

int check_click_random_button();

#endif
