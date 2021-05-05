#ifndef _VGA_H
#define _VGA_H

#include "lib.h"
#include "vbe.h"

#define VGA_DIMX 1024
#define VGA_DIMY 768
#define BACKGROUND 0x3E9092

volatile int need_update;
typedef struct ARGB {
    uint8_t b;
    uint8_t g;
    uint8_t r;
    uint8_t a;
} ARGB;

void init_vga();

void show_screen();
void clear_screen();

void patch_mouse(int x, int y);

void Pcopy(int x, int y, uint32_t clr);
void Pdraw(int x, int y, uint32_t clr);
void Rdraw(int w, int h, int x, int y, uint32_t clr);

#endif
