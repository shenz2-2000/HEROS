#include "gui.h"

char font8x8_basic[128][8] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},
        { 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},
        { 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},
        { 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},
        { 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},
        { 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},
        { 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},
        { 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},
        { 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},
        { 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},
        { 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},
        { 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},
        { 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},
        { 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},
        { 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},
        { 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},
        { 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},
        { 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},
        { 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},
        { 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},
        { 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},
        { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},
        { 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},
        { 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},
        { 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},
        { 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},
        { 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},
        { 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},
        { 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},
        { 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},
        { 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},
        { 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},
        { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},
        { 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},
        { 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},
        { 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},
        { 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
        { 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},
        { 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},
        { 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},
        { 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},
        { 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},
        { 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},
        { 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},
        { 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},
        { 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},
        { 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},
        { 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
        { 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},
        { 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},
        { 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
        { 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},
        { 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},
        { 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},
        { 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},
        { 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},
        { 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},
        { 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
        { 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},
        { 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},
        { 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},
        { 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},
        { 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},
        { 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},
        { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},
        { 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},
        { 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
        { 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},
        { 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},
        { 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},
        { 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},
        { 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},
        { 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},
        { 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},
        { 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},
        { 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},
        { 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},
        { 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},
        { 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},
        { 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},
        { 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},
        { 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},
        { 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},
        { 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},
        { 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},
        { 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},
        { 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},
        { 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
};

void init_gui() {
    int i = 0;
    init_vga();


    Rdraw(VGA_DIMX, 36, 0, VGA_DIMY-36, 0xAFEEEE);

    setup_status_bar();

}

void setup_status_bar(){
    Rdraw(VGA_DIMX, 36, 0, VGA_DIMY-36, 0xAFEEEE);
    char status_bar[26] = "UTC+0:2021-00-00 00:00:00";
    status_bar[11] = month/10+48;
    status_bar[12] = month%10+48;
    status_bar[14] = day/10+48;
    status_bar[15] = day%10+48;
    status_bar[17] = hour/10+48;
    status_bar[18] = hour%10+48;
    status_bar[20] = mins/10+48;
    status_bar[21] = mins%10+48;
    status_bar[23] = sec/10+48;
    status_bar[24] = sec%10+48;
    render_string(5, VGA_DIMY-30 , "Welcome to Group14-OS", 0x0000);
    render_string(VGA_DIMX-400, VGA_DIMY-30 , status_bar, 0x0000);
    need_update = 1;
}
//
void render_font(int x_start, int y_start, char ch, uint32_t color) {
    char* font = (char*)font8x8_basic[(uint8_t)(ch)];
    int x,y;
    int set;
    for (y=0; y < 8; y++) {
        for (x=0; x < 8; x++) {
            set = font[y] & 1 << x;
            if(set) {
                Pdraw(x_start + x, y_start + y, color);
            }
        }
    }
}

void render_string(int x_start, int y_start, char* string, uint32_t color) {
    int i;
    for(i = 0; i < strlen(string); i++) {
        render_font(x_start + i*8, y_start, string[i], color);
    }
}

void render_window(int x, int y, int width, int height, char* title, uint8_t is_focus) {
    Rdraw(width, height, x, y,0xCDCECF);
    if(is_focus) {
        Rdraw(width - 4, 20,x + 2, y + 2,  0x000E8C);
        render_string(x+16, y+8, title, 0xFFFFFF);
    } else {
        Rdraw(width - 4, 20, x + 2, y + 2, 0x929292);
        render_string(x+16, y+8, title, 0xC7C7C7);
    }
}
