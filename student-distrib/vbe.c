// reference1: https://forum.osdev.org/viewtopic.php?f=1&t=30884
// reference2: https://wiki.osdev.org/Bochs_VBE_Extensions

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

// BgaWriteRegister
void BgaWriteRegister(unsigned short IndexValue, unsigned short DataValue)
{
    outw(IndexValue, VBE_DISPI_IOPORT_INDEX);
    outw(DataValue, VBE_DISPI_IOPORT_DATA);
}

void veb_clear() {
    BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
}

void init_vbe(uint16_t res_x, uint16_t res_y, uint16_t bpp)
{
    BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    BgaWriteRegister(VBE_DISPI_INDEX_XRES, res_x);
    BgaWriteRegister(VBE_DISPI_INDEX_YRES, res_y);
    BgaWriteRegister(VBE_DISPI_INDEX_BPP, bpp);
    BgaWriteRegister(VBE_DISPI_INDEX_FB_BASE_HI, REG_VBE >> 16);
    BgaWriteRegister(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED);
}
