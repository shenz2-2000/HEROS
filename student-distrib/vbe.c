// reference: https://wiki.osdev.org/VGA_Hardware#Memory_Layout_in_16-color_graphics_modes and linux v5 source code

#include "vbe.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA  0x01CF
#define VBE_DISPI_INDEX_XRES            0x1
#define VBE_DISPI_INDEX_YRES            0x2
#define VBE_DISPI_INDEX_BPP             0x3
#define VBE_DISPI_INDEX_ENABLE          0x4
#define VBE_DISPI_DISABLED              0x00
#define VBE_DISPI_ENABLED               0x01
#define VBE_DISPI_INDEX_FB_BASE_HI      0x0B

void vbe_write(uint16_t index, uint16_t value)
{
    outw(index, VBE_DISPI_IOPORT_INDEX);
    outw(value, VBE_DISPI_IOPORT_DATA);
}

void veb_clear() {
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
}

void init_vbe(uint16_t xres, uint16_t yres, uint16_t bpp)
{
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    vbe_write(VBE_DISPI_INDEX_XRES, xres);
    vbe_write(VBE_DISPI_INDEX_YRES, yres);
    vbe_write(VBE_DISPI_INDEX_BPP, bpp);
    vbe_write(VBE_DISPI_INDEX_FB_BASE_HI, REG_VBE >> 16);
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED);
}
