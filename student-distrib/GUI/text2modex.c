// This file is used to transform the original textmode information to the new terminal information
// Created by shenz2 on 2021/5/3.
//

#include "text2modex.h"
#include "modex.h"

int draw_text_buffer[MODEX_TER_ROWS*MODEX_TER_COLS*FONT_WIDTH*FONT_HEIGHT]; // Indicate whether each pixel is zero or one
#define VIDEO       0xB8000
static char* video_cache = (char *)VIDEO;
// TODO: Change the information here
/*
 * text_to_graphic
 *   DESCRIPTION: Convert the original information in VIDEO to the graphics mode
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 */

void text_to_graphic() {
    // The input state is a boolean with 16*320
    int i, j, k, l;
    char cur_char;
    for (i = 0; i < MODEX_TER_ROWS; ++i) {
        for (j = 0; j < MODEX_TER_COLS; ++j)
            for (k = 0; k < FONT_HEIGHT; ++k)
                for (l = 0; l < FONT_WIDTH; ++l) {
                    cur_char = *(uint8_t *)(video_mem + ((NUM_COLS * i + j) << 1))
                    draw_text_buffer[i * MODEX_TER_COLS * FONT_HEIGHT * FONT_WIDTH + k * MODEX_TER_COLS + j * FONT_WIDTH + l] =
                            (font8x8[cur_char][k] & (1 << (8 - l))) > 0;
                }
    }
    draw_textmode_terminal();
}
