#ifndef _VBE_H
#define _VBE_H

#include "lib.h"
#include "types.h"

#define REG_VBE 0xFD000000 // both virtual and physical

void init_vbe(uint16_t xres, uint16_t yres, uint16_t bpp);

#endif
