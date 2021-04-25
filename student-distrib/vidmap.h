#ifndef _VIDMAP_H
#define _VIDMAP_H

// TODO: delete this before push!
#define MAX_TERMINAL 3

#define VM_INDEX      0xB8000
#define VM_PTE        0xB8

#define BITS_4K        4096     // 0x1000
#define BITS_4M        4194304  // 0x40000

// terminal status
#define TERMINAL_ON     MAX_TERMINAL+2  // avoid interference
#define TERMINAL_OFF    MAX_TERMINAL+1

#endif