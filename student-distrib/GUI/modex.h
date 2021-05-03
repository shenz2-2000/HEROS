//
// Created by BOURNE on 2021/5/2.
//

#ifndef MIAO_MODEX_H
#define MIAO_MODEX_H

#include "text2modex.h"


// TODO: need modification here
#define IMAGE_X_DIM     320   /* pixels; must be divisible by 4             */
#define IMAGE_Y_DIM     200 -16  /* pixels                                     */
#define IMAGE_X_WIDTH   (IMAGE_X_DIM / 4)          /* addresses (bytes)     */
#define SCROLL_X_DIM    IMAGE_X_DIM                /* full image width      */
// TODO:
#define SCROLL_Y_DIM    IMAGE_Y_DIM - 16             /* full image width      */
#define SCROLL_X_WIDTH  (IMAGE_X_DIM / 4)          /* addresses (bytes)     */
#define SCROLL_SIZE             (SCROLL_X_WIDTH * SCROLL_Y_DIM)
#define SCREEN_SIZE             (SCROLL_SIZE * 4 + 1)
#define BUILD_BUF_SIZE          (SCREEN_SIZE + 20000)
#define BUILD_BASE_INIT         ((BUILD_BUF_SIZE - SCREEN_SIZE) / 2)

// register numbers
#define NUM_SEQUENCER_REGS      5
#define NUM_CRTC_REGS           25
#define NUM_GRAPHICS_REGS       9
#define NUM_ATTR_REGS           22

// memory fence
#define MEM_FENCE_WIDTH         256
#define MEM_FENCE_MAGIC         0xF3

/* Mode X and general VGA parameters */
#define VID_MEM_SIZE            131072
#define MODE_X_MEM_SIZE         65536
#define NUM_SEQUENCER_REGS      5
#define NUM_CRTC_REGS           25
#define NUM_GRAPHICS_REGS       9
#define NUM_ATTR_REGS           22

extern int set_mode_X(
        void (*horiz_fill_fn)(int, int, unsigned char[SCROLL_X_DIM]),
        void (*vert_fill_fn)(int, int, unsigned char[SCROLL_Y_DIM]));

extern void test_video_with_garbage();
extern  void show_screen();
extern  void update_four_planes();
void draw_textmode_terminal();
void compute_status_bar();
extern char font8x8[128][8];
extern int draw_text_buffer[MODEX_TER_ROWS*MODEX_TER_COLS*FONT_WIDTH*FONT_HEIGHT];
#endif //MIAO_MODEX_H
