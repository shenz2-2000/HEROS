
#include "vga.h"
#include "mouse_driver.h"

ARGB* vbe_mem = (ARGB*)REG_VBE;
static ARGB fbuf[VGA_DIMY][VGA_DIMX] __attribute__((aligned(32)));


static void cpbuf(void* tar,void* buf, int nbytes) {
    asm volatile ("                                             \n\
        cld                                                     \n\
        rep movsl    /*  M[ESI] -> M[EDI] */                    \n\
        "
    :
    : "S"(buf), "D"(tar), "c"(nbytes)
    : "eax", "memory"
    );
}

void clear_screen() {
    Rdraw(VGA_DIMX, VGA_DIMY, 0, 0, 0);
    show_screen();
}

void show_screen() {
    if (need_update == 1) {
        // if (memcpy(vbe_mem, fbuf, VGA_DIMY * VGA_DIMX) == vbe_mem) need_update = 0;
         cpbuf(vbe_mem, fbuf, VGA_DIMY * VGA_DIMX);
         need_update = 0;
    }

//    erase_mouse();
//    render_mouse(mouse_x,mouse_y);


}

void init_vga() {
    init_vbe(VGA_DIMX, VGA_DIMY, 32);
    // Rdraw(VGA_DIMX, VGA_DIMY, 0, 0, 0xD9A179);
    int i, j;
    for(i = 0; i < VGA_DIMX; i++)
        for(j = 0; j < VGA_DIMY; j++)
            Pdraw(i, j, 0xD9A179+i+j*2);
}

void Rdraw(int w, int h, int x, int y, uint32_t clr) {
    int i, j;
    for(i = 0; i < w; i++)
        for(j = 0; j < h; j++)
            Pdraw(x + i, y + j, clr);
}

void Pdraw(int x, int y, uint32_t clr) {
    if( x < 0 || y < 0 || x >= VGA_DIMX || y >= VGA_DIMY)
        return;
    fbuf[y][x].r = ((clr >> 16) & 0xFF);
    fbuf[y][x].g = ((clr >> 8) & 0xFF);
    fbuf[y][x].b = (clr & 0xFF);
    need_update = 1;
}