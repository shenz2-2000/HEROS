//
// Created by BOURNE on 2021/5/2.
//

#include "modex.h"
#include "../lib.h"
#include "../x86_desc.h"
#include "../page_lib.h"
#include "blocks.h"
#include "maze.h"
#include "text2modex.h"
// ------------------------------Macros and Function Declaration------------------------------------



// buffer and important variables
static unsigned char build[BUILD_BUF_SIZE + 2 * MEM_FENCE_WIDTH];
static int img3_off;                /* offset of upper left pixel   */
static unsigned char* img3;         /* pointer to upper left pixel  */
static int show_x, show_y;          /* logical view coordinates     */

// displayed video memory variables
static unsigned char* mem_image;    /* pointer to start of video memory */
static unsigned short target_img;   /* offset of displayed screen image */


// for maze
static unsigned char maze[2 * MAZE_MAX_X_DIM * (2 * MAZE_MAX_Y_DIM + 3) + 1];
static int maze_x_dim;          /* horizontal dimension of maze */
static int maze_y_dim;          /* vertical dimension of maze   */
static int n_fruits;            /* number of fruits in maze     */
static int exit_x, exit_y;      /* lattice point of maze exit   */

#define MAZE_INDEX(a,b) ((a) + ((b) + 1) * maze_x_dim * 2)


// our main buffer
static unsigned char main_buffer[SCROLL_X_DIM * SCROLL_Y_DIM];
static unsigned char plane0[SCROLL_X_DIM * SCROLL_Y_DIM / 4];
static unsigned char plane1[SCROLL_X_DIM * SCROLL_Y_DIM / 4];
static unsigned char plane2[SCROLL_X_DIM * SCROLL_Y_DIM / 4];
static unsigned char plane3[SCROLL_X_DIM * SCROLL_Y_DIM / 4];

static unsigned char status_bar_buffer[16*80];
static unsigned char status_plane0[16*80/4];
static unsigned char status_plane1[16*80/4];
static unsigned char status_plane2[16*80/4];
static unsigned char status_plane3[16*80/4];


// two function pointers
static void (*horiz_line_fn)(int, int, unsigned char[SCROLL_X_DIM]);
static void (*vert_line_fn)(int, int, unsigned char[SCROLL_Y_DIM]);

//static int open_memory_and_ports();
static void VGA_blank(int blank_bit);
static void set_seq_regs_and_reset(unsigned short table[NUM_SEQUENCER_REGS], unsigned char val);
static void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]);
static void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]);
static void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]);
static void fill_palette();
static void copy_image(unsigned char* img, unsigned short scr_addr);
static void set_memory_for_modex();
void fill_horiz_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]);
static unsigned char* find_block(int x, int y);


// ---------------------------------------Useful arrays---------------------------------------------




// The three palette for wall and player
static unsigned  char  player_palette[10][3] = {
        {0x00,0x00,0xCD},
        {0x00,0xFF,0x99},
        {0x56,0x12,0x2A},
        {0x43,0x12,0x56},
        {0x17,0x56,0x12},
        {0x12,0x53,0x56},
        {0x12,0x18,0x56},
        {0x12,0x56,0x51},
        {0x36,0x12,0x56},
        {0x1F,0x3E,0x27},
};

static unsigned  char  wall_content_palette[10][3] = {
        {0x00,0x00,0xCD},
        {0x00,0xFF,0x99},
        {0x56,0x12,0x2A},
        {0x43,0x12,0x56},
        {0x17,0x56,0x12},
        {0x12,0x53,0x56},
        {0x12,0x18,0x56},
        {0x12,0x56,0x51},
        {0x36,0x12,0x56},
        {0x1F,0x3E,0x27},
};

static unsigned  char  wall_edge_palette[10][3] = {
        {0x00,0x00,0xCD},
        {0x00,0xFF,0x99},
        {0x56,0xA2,0x2A},
        {0x49,0x12,0x56},
        {0x17,0x56,0x12},
        {0x12,0x53,0x56},
        {0x12,0x18,0x2A},
        {0x12,0x66,0x51},
        {0x00,0x12,0x56},
        {0x1F,0x3E,0x27},
};


/* VGA register settings for mode X */
static unsigned short mode_X_seq[NUM_SEQUENCER_REGS] = {
        0x0100, 0x2101, 0x0F02, 0x0003, 0x0604
};

//static unsigned short mode_X_CRTC[NUM_CRTC_REGS] = {
//    0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
//    0x0008, 0x4109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
//    0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
//    0xB618
//};

/*here we need to modify MAX_SCAN_LINE(09) to 01 (bit 9 of line compare)
                         LINE_COMPARE_REGISTER(18) to 6C (182 * 2 - 1 - 256)
  Then we add a status bar (height 16 + 2) to the line 182 (200 - 18) at the bottom of screen*/

/*The reason why is 6B18 is that VGA will scan each line twice, so 182 means the 363th and 364th scan.
  we put 363 - 256 in port 18 to ensure the correct position of status bar*/
/*The reason why is 0109 is that the 9th bit of line compare field is in max scan line. We need to
  clear that bit*/
static unsigned short mode_X_CRTC[NUM_CRTC_REGS] = {
        0x5F00, 0x4F01, 0x5002, 0x8203, 0x5404, 0x8005, 0xBF06, 0x1F07,
        0x0008, 0x0109, 0x000A, 0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
        0x9C10, 0x8E11, 0x8F12, 0x2813, 0x0014, 0x9615, 0xB916, 0xE317,
        0x6F18
};

static unsigned char mode_X_attr[NUM_ATTR_REGS * 2] = {
        0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
        0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
        0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
        0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
        0x10, 0x41, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x00,
        0x14, 0x00, 0x15, 0x00
};

static unsigned short mode_X_graphics[NUM_GRAPHICS_REGS] = {
        0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x4005, 0x0506, 0x0F07,
        0xFF08
};

/* VGA register settings for text mode 3 (color text) */
static unsigned short text_seq[NUM_SEQUENCER_REGS] = {
        0x0100, 0x2001, 0x0302, 0x0003, 0x0204
};

static unsigned short text_CRTC[NUM_CRTC_REGS] = {
        0x5F00, 0x4F01, 0x5002, 0x8203, 0x5504, 0x8105, 0xBF06, 0x1F07,
        0x0008, 0x4F09, 0x0D0A, 0x0E0B, 0x000C, 0x000D, 0x000E, 0x000F,
        0x9C10, 0x8E11, 0x8F12, 0x2813, 0x1F14, 0x9615, 0xB916, 0xA317,
        0xFF18
};
static unsigned char text_attr[NUM_ATTR_REGS * 2] = {
        0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x03,
        0x04, 0x04, 0x05, 0x05, 0x06, 0x06, 0x07, 0x07,
        0x08, 0x08, 0x09, 0x09, 0x0A, 0x0A, 0x0B, 0x0B,
        0x0C, 0x0C, 0x0D, 0x0D, 0x0E, 0x0E, 0x0F, 0x0F,
        0x10, 0x0C, 0x11, 0x00, 0x12, 0x0F, 0x13, 0x08,
        0x14, 0x00, 0x15, 0x00
};
static unsigned short text_graphics[NUM_GRAPHICS_REGS] = {
        0x0000, 0x0001, 0x0002, 0x0003, 0x0004, 0x1005, 0x0E06, 0x0007,
        0xFF08
};

static unsigned char original_palette[64][3] = {
        { 0x00, 0x00, 0x00 },{ 0xFF, 0xFF, 0xFF },   /* palette 0x00 - 0x0F    */
        { 0x00, 0x2A, 0x00 },{ 0x00, 0x2A, 0x2A },   /* basic VGA colors       */
        { 0x2A, 0x00, 0x00 },{ 0x2A, 0x00, 0x2A },
        { 0x2A, 0x15, 0x00 },{ 0x2A, 0x2A, 0x2A },
        { 0x15, 0x15, 0x15 },{ 0x15, 0x15, 0x3F },
        { 0x15, 0x3F, 0x15 },{ 0x15, 0x3F, 0x3F },
        { 0x3F, 0x15, 0x15 },{ 0x3F, 0x15, 0x3F },
        { 0x3F, 0x3F, 0x15 },{ 0x3F, 0x3F, 0x3F },
        { 0x00, 0x00, 0x00 },{ 0x05, 0x05, 0x05 },   /* palette 0x10 - 0x1F    */
        { 0x08, 0x08, 0x08 },{ 0x0B, 0x0B, 0x0B },   /* VGA grey scale         */
        { 0x0E, 0x0E, 0x0E },{ 0x11, 0x11, 0x11 },
        { 0x14, 0x14, 0x14 },{ 0x18, 0x18, 0x18 },
        { 0x1C, 0x1C, 0x1C },{ 0x20, 0x20, 0x20 },
        { 0x24, 0x24, 0x24 },{ 0x28, 0x28, 0x28 },
        { 0x2D, 0x2D, 0x2D },{ 0x32, 0x32, 0x32 },
        { 0x38, 0x38, 0x38 },{ 0x3F, 0x3F, 0x3F },
        { 0x3F, 0x3F, 0x3F },{ 0x3F, 0x3F, 0x3F },   /* palette 0x20 - 0x2F    */
        { 0x00, 0x00, 0x3F },{ 0x00, 0x00, 0x00 },   /* wall and player colors */
        { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },   /* END OF WALL AND PLAYER */
        { 0x10, 0x08, 0x00 },{ 0x18, 0x0C, 0x00 },   /* palette 0x30 - 0x3F    */
        { 0x20, 0x10, 0x00 },{ 0x28, 0x14, 0x00 },   /* browns for maze floor  */
        { 0x30, 0x18, 0x00 },{ 0x38, 0x1C, 0x00 },
        { 0x3F, 0x20, 0x00 },{ 0x3F, 0x20, 0x10 },
        { 0x20, 0x18, 0x10 },{ 0x28, 0x1C, 0x10 },
        { 0x3F, 0x20, 0x10 },{ 0x38, 0x24, 0x10 },
        { 0x3F, 0x28, 0x10 },{ 0x3F, 0x2C, 0x10 },
        { 0x3F, 0x30, 0x10 },{ 0x3F, 0x20, 0x10 }
};

// -----------------------------------------------------Functions----------------------------------------------------
void fill_horiz_buffer(int x, int y, unsigned char buf[SCROLL_X_DIM]) {
    int map_x, map_y;     /* maze lattice point of the first block on line */
    int sub_x, sub_y;     /* sub-block address                             */
    int idx;              /* loop index over pixels in the line            */
    unsigned char* block; /* pointer to current maze block image           */

    /* Find the maze lattice point and the pixel address within that block. */
    map_x = x / BLOCK_X_DIM;
    map_y = y / BLOCK_Y_DIM;
    sub_x = x - map_x * BLOCK_X_DIM;
    sub_y = y - map_y * BLOCK_Y_DIM;

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_X_DIM; ) {

        /* Find address of block to be drawn. */
//        block = find_block(map_x++, map_y) + sub_y * BLOCK_X_DIM + sub_x;

        /* Write block colors from one line into buffer. */
        for (; idx < SCROLL_X_DIM && sub_x < BLOCK_X_DIM; idx++, sub_x++)
            buf[idx] = 1;

        /*
         * All subsequent blocks are copied starting from the left side
         * of the block.
         */
        sub_x = 0;
    }
}


void fill_vert_buffer(int x, int y, unsigned char buf[SCROLL_Y_DIM]) {
    int map_x, map_y;     /* maze lattice point of the first block on line */
    int sub_x, sub_y;     /* sub-block address                             */
    int idx;              /* loop index over pixels in the line            */
    unsigned char* block; /* pointer to current maze block image           */

    /* Find the maze lattice point and the pixel address within that block. */
    map_x = x / BLOCK_X_DIM;
    map_y = y / BLOCK_Y_DIM;
    sub_x = x - map_x * BLOCK_X_DIM;
    sub_y = y - map_y * BLOCK_Y_DIM;

    /* Loop over pixels in line. */
    for (idx = 0; idx < SCROLL_Y_DIM; ) {

//        /* Find address of block to be drawn. */
//        block = find_block(map_x, map_y++) + sub_y * BLOCK_X_DIM + sub_x;

        /* Write block colors from one line into buffer. */
        for (; idx < SCROLL_Y_DIM && sub_y < BLOCK_Y_DIM;
               idx++, sub_y++)
            buf[idx] = 1;

        /*
         * All subsequent blocks are copied starting from the top
         * of the block.
         */
        sub_y = 0;
    }

    return;
}






static void set_memory_for_modex(){

    uint32_t i;
    uint32_t j;

    PDE_4K_set(page_directory+2,(uint32_t) modex_page_table,1,1,1);

    for(i = 160; i <= 191; i++){
        j = i << 12;
        PTE_set(modex_page_table+i,j,1,1,1);
    }

    mem_image = (uint8_t*) 9043968;


}


/*
 * copy_image
 *   DESCRIPTION: Copy one plane of a screen from the build buffer to the
 *                video memory.
 *   INPUTS: img -- a pointer to a single screen plane in the build buffer
 *           scr_addr -- the destination offset in video memory
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: copies a plane from the build buffer to video memory
 */
static void copy_image(unsigned char* img, unsigned short scr_addr) {
    /*
     * memcpy is actually probably good enough here, and is usually
     * implemented using ISA-specific features like those below,
     * but the code here provides an example of x86 string moves
     */
    asm volatile ("                                             \n\
        cld                                                     \n\
        movl $16000,%%ecx                                       \n\
        rep movsb    /* copy ECX bytes from M[ESI] to M[EDI] */ \n\
        "
    : /* no outputs */
    : "S"(img), "D"(mem_image + scr_addr)
    : "eax", "ecx", "memory"
    );
}




/*
 * macro used to target a specific video plane or planes when writing
 * to video memory in mode X; bits 8-11 in the mask_hi_bits enable writes
 * to planes 0-3, respectively
 */
#define SET_WRITE_MASK(mask_hi_bits)                                \
do {                                                                \
    asm volatile ("                                               \n\
        movw $0x03C4, %%dx    /* set write mask */                \n\
        movb $0x02, %b0                                           \n\
        outw %w0, (%%dx)                                          \n\
        "                                                           \
        : /* no outputs */                                          \
        : "a"((mask_hi_bits))                                       \
        : "edx", "memory"                                           \
    );                                                              \
} while (0)

/* macro used to write a byte to a port */
#define OUTB(port, val)                                             \
do {                                                                \
    asm volatile ("outb %b1, (%w0)"                                 \
        : /* no outputs */                                          \
        : "d"((port)), "a"((val))                                   \
        : "memory", "cc"                                            \
    );                                                              \
} while (0)

/* macro used to write two bytes to two consecutive ports */
#define OUTW(port, val)                                             \
do {                                                                \
    asm volatile ("outw %w1, (%w0)"                                 \
        : /* no outputs */                                          \
        : "d"((port)), "a"((val))                                   \
        : "memory", "cc"                                            \
    );                                                              \
} while (0)

/* macro used to write an array of two-byte values to two consecutive ports */
#define REP_OUTSW(port, source, count)                              \
do {                                                                \
    asm volatile ("                                               \n\
        1: movw 0(%1), %%ax                                       \n\
        outw %%ax, (%w2)                                          \n\
        addl $2, %1                                               \n\
        decl %0                                                   \n\
        jne 1b                                                    \n\
        "                                                           \
        : /* no outputs */                                          \
        : "c"((count)), "S"((source)), "d"((port))                  \
        : "eax", "memory", "cc"                                     \
    );                                                              \
} while (0)

/* macro used to write an array of one-byte values to two consecutive ports */
#define REP_OUTSB(port, source, count)                              \
do {                                                                \
    asm volatile ("                                               \n\
        1: movb 0(%1), %%al                                       \n\
        outb %%al, (%w2)                                          \n\
        incl %1                                                   \n\
        decl %0                                                   \n\
        jne 1b                                                    \n\
        "                                                           \
        : /* no outputs */                                          \
        : "c"((count)), "S"((source)), "d"((port))                  \
        : "eax", "memory", "cc"                                     \
    );                                                              \
} while (0)

// TODO: understand and modify
/*
 * clear_screens
 *   DESCRIPTION: Fills the video memory with zeroes.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: fills all 256kB of VGA video memory with zeroes
 */
void clear_screens() {
    /* Write to all four planes at once. */
    SET_WRITE_MASK(0x0F00);

    /* Set 64kB to zero (times four planes = 256kB). */
    memset(mem_image, 0, MODE_X_MEM_SIZE);
}



// TODO: modify here
int set_mode_X(void (*horiz_fill_fn)(int, int, unsigned char[SCROLL_X_DIM]),
               void (*vert_fill_fn)(int, int, unsigned char[SCROLL_Y_DIM])) {

    /* loop index for filling memory fence with magic numbers */
    int i;

//    /*
//     * Record callback functions for obtaining horizontal and vertical
//     * line images.
//     */
//    if (horiz_fill_fn == NULL || vert_fill_fn == NULL)
//        return -1;
//    horiz_line_fn = horiz_fill_fn;
//    vert_line_fn = vert_fill_fn;
//
//    /* Initialize the logical view window to position (0,0). */
//    show_x = show_y = 0;
//    img3_off = BUILD_BASE_INIT;
//    img3 = build + img3_off + MEM_FENCE_WIDTH;
//
//    /* Set up the memory fence on the build buffer. */
//    for (i = 0; i < MEM_FENCE_WIDTH; i++) {
//        build[i] = MEM_FENCE_MAGIC;
//        build[BUILD_BUF_SIZE + MEM_FENCE_WIDTH + i] = MEM_FENCE_MAGIC;
//    }

    /* One display page goes at the start of video memory. */
    /* we spare 16*80 (18 is the height of the text, 80 is the width of status bar) memory locations for status bar */
    target_img = 0x0500;
//     target_img = 0;


    /* Map video memory and obtain permission for VGA port access. */
//    if (open_memory_and_ports() == -1)
//        return -1;
    set_memory_for_modex();

    /*
     * The code below was produced by recording a call to set mode 0013h
     * with display memory clearing and a windowed frame buffer, then
     * modifying the code to set mode X instead.  The code was then
     * generalized into functions...
     *
     * modifications from mode 13h to mode X include...
     *   Sequencer Memory Mode Register: 0x0E to 0x06 (0x3C4/0x04)
     *   Underline Location Register   : 0x40 to 0x00 (0x3D4/0x14)
     *   CRTC Mode Control Register    : 0xA3 to 0xE3 (0x3D4/0x17)
     */

    VGA_blank(1);                               /* blank the screen      */
    set_seq_regs_and_reset(mode_X_seq, 0x63);   /* sequencer registers   */
    set_CRTC_registers(mode_X_CRTC);            /* CRT control registers */
    set_attr_registers(mode_X_attr);            /* attribute registers   */
    set_graphics_registers(mode_X_graphics);    /* graphics registers    */
    fill_palette();                             /* palette colors        */
    clear_screens();                            /* zero video memory     */
    VGA_blank(0);                               /* unblank the screen    */

    test_video_with_garbage();
    show_screen();
    /* Return success. */
    return 0;
}


/*
 * VGA_blank
 *   DESCRIPTION: Blank or unblank the VGA display.
 *   INPUTS: blank_bit -- set to 1 to blank, 0 to unblank
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void VGA_blank(int blank_bit) {
    /*
     * Move blanking bit into position for VGA sequencer register
     * (index 1).
     */
    blank_bit = ((blank_bit & 1) << 5);

    asm volatile ("                                                    \n\
        movb $0x01, %%al         /* Set sequencer index to 1.       */ \n\
        movw $0x03C4, %%dx                                             \n\
        outb %%al, (%%dx)                                              \n\
        incw %%dx                                                      \n\
        inb  (%%dx), %%al        /* Read old value.                 */ \n\
        andb $0xDF, %%al         /* Calculate new value.            */ \n\
        orl  %0, %%eax                                                 \n\
        outb %%al, (%%dx)        /* Write new value.                */ \n\
        movw $0x03DA, %%dx       /* Enable display (0x20->P[0x3C0]) */ \n\
        inb  (%%dx), %%al        /* Set attr reg state to index.    */ \n\
        movw $0x03C0, %%dx       /* Write index 0x20 to enable.     */ \n\
        movb $0x20, %%al                                               \n\
        outb %%al, (%%dx)                                              \n\
        "
    :
    : "g"(blank_bit)
    : "eax", "edx", "memory"
    );
}

/*
 * set_seq_regs_and_reset
 *   DESCRIPTION: Set VGA sequencer registers and miscellaneous output
 *                register; array of registers should force a reset of
 *                the VGA sequencer, which is restored to normal operation
 *                after a brief delay.
 *   INPUTS: table -- table of sequencer register values to use
 *           val -- value to which miscellaneous output register should be set
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void set_seq_regs_and_reset(unsigned short table[NUM_SEQUENCER_REGS], unsigned char val) {
    /*
     * Dump table of values to sequencer registers.  Includes forced reset
     * as well as video blanking.
     */
    REP_OUTSW(0x03C4, table, NUM_SEQUENCER_REGS);

    /* Delay a bit... */
    {volatile int ii; for (ii = 0; ii < 10000; ii++);}

    /* Set VGA miscellaneous output register. */
    OUTB(0x03C2, val);

    /* Turn sequencer on (array values above should always force reset). */
    OUTW(0x03C4, 0x0300);
}

/*
 * set_CRTC_registers
 *   DESCRIPTION: Set VGA cathode ray tube controller (CRTC) registers.
 *   INPUTS: table -- table of CRTC register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void set_CRTC_registers(unsigned short table[NUM_CRTC_REGS]) {
    /* clear protection bit to enable write access to first few registers */
    OUTW(0x03D4, 0x0011);
    REP_OUTSW(0x03D4, table, NUM_CRTC_REGS);
}

/*
 * set_attr_registers
 *   DESCRIPTION: Set VGA attribute registers.  Attribute registers use
 *                a single port and are thus written as a sequence of bytes
 *                rather than a sequence of words.
 *   INPUTS: table -- table of attribute register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void set_attr_registers(unsigned char table[NUM_ATTR_REGS * 2]) {
    /* Reset attribute register to write index next rather than data. */
    asm volatile ("inb (%%dx),%%al"
    :
    : "d"(0x03DA)
    : "eax", "memory"
    );
    REP_OUTSB(0x03C0, table, NUM_ATTR_REGS * 2);
}

/*
 * set_graphics_registers
 *   DESCRIPTION: Set VGA graphics registers.
 *   INPUTS: table -- table of graphics register values to use
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: none
 */
static void set_graphics_registers(unsigned short table[NUM_GRAPHICS_REGS]) {
    REP_OUTSW(0x03CE, table, NUM_GRAPHICS_REGS);
}

/*
 * fill_palette
 *   DESCRIPTION: Fill VGA palette with necessary colors for the maze game.
 *                Only the first 64 (of 256) colors are written.
 *   INPUTS: none
 *   OUTPUTS: none
 *   RETURN VALUE: none
 *   SIDE EFFECTS: changes the first 64 palette colors
 */
static void fill_palette() {
    /* 6-bit RGB (red, green, blue) values for first 64 colors */
    static unsigned char palette_RGB[64][3] = {
            { 0x00, 0x00, 0x00 },{ 0xDC, 0x14, 0x3C },   /* palette 0x00 - 0x0F    */
            { 0x00, 0x2A, 0x00 },{ 0x00, 0x00, 0xFF },   /* basic VGA colors       */
            { 0x2A, 0x00, 0x00 },{ 0x2A, 0x00, 0x2A },
            { 0x2A, 0x15, 0x00 },{ 0x2A, 0x2A, 0x2A },
            { 0x15, 0x15, 0x15 },{ 0x15, 0x15, 0x3F },
            { 0x15, 0x3F, 0x15 },{ 0x15, 0x3F, 0x3F },
            { 0x3F, 0x15, 0x15 },{ 0x3F, 0x15, 0x3F },
            { 0x3F, 0x3F, 0x15 },{ 0x3F, 0x3F, 0x3F },
            { 0x00, 0x00, 0x00 },{ 0x05, 0x05, 0x05 },   /* palette 0x10 - 0x1F    */
            { 0x08, 0x08, 0x08 },{ 0x0B, 0x0B, 0x0B },   /* VGA grey scale         */
            { 0x0E, 0x0E, 0x0E },{ 0x11, 0x11, 0x11 },
            { 0x14, 0x14, 0x14 },{ 0x18, 0x18, 0x18 },
            { 0x1C, 0x1C, 0x1C },{ 0x20, 0x20, 0x20 },
            { 0x24, 0x24, 0x24 },{ 0x28, 0x28, 0x28 },
            { 0x2D, 0x2D, 0x2D },{ 0x32, 0x32, 0x32 },
            { 0x38, 0x38, 0x38 },{ 0x3F, 0x3F, 0x3F },
            { 0x3F, 0x3F, 0x3F },{ 0x3F, 0x3F, 0x3F },   /* palette 0x20 - 0x2F    */
            { 0x00, 0x00, 0x3F },{ 0x00, 0x00, 0x00 },   /* wall and player colors */
            { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },
            { 0x00, 0x00, 0x00 },{ 0x00, 0x00, 0x00 },   /* END OF WALL AND PLAYER */
            { 0x10, 0x08, 0x00 },{ 0x18, 0x0C, 0x00 },   /* palette 0x30 - 0x3F    */
            { 0x20, 0x10, 0x00 },{ 0x28, 0x14, 0x00 },   /* browns for maze floor  */
            { 0x30, 0x18, 0x00 },{ 0x38, 0x1C, 0x00 },
            { 0x3F, 0x20, 0x00 },{ 0x3F, 0x20, 0x10 },
            { 0x20, 0x18, 0x10 },{ 0x28, 0x1C, 0x10 },
            { 0x3F, 0x20, 0x10 },{ 0x38, 0x24, 0x10 },
            { 0x3F, 0x28, 0x10 },{ 0x3F, 0x2C, 0x10 },
            { 0x3F, 0x30, 0x10 },{ 0x3F, 0x20, 0x10 }
    };

    /* Start writing at color 0. */
    OUTB(0x03C8, 0x00);

    /* Write all 64 colors from array. */
    REP_OUTSB(0x03C9, palette_RGB, 64 * 3);
}




void show_screen() {

    update_four_planes();

    SET_WRITE_MASK(1 << (0 + 8));
    copy_image(plane0, target_img);
    SET_WRITE_MASK(1<<  (1 + 8));
    copy_image(plane1, target_img);
    SET_WRITE_MASK(1<<  (2 + 8));
    copy_image(plane2, target_img);
    SET_WRITE_MASK(1<<  (3 + 8));
    copy_image(plane3, target_img);

    /*
     * Change the VGA registers to point the top left of the screen
     * to the video memory that we just filled.
     */
    OUTW(0x03D4, (target_img & 0xFF00) | 0x0C);
    OUTW(0x03D4, ((target_img & 0x00FF) << 8) | 0x0D);
}




void test_video_with_garbage(){
    int i,j;
    unsigned char* addr;
    addr = mem_image;
    for(i = 0; i < SCROLL_X_DIM; i++){
        for(j = 0; j < SCROLL_Y_DIM+16; j++){
            main_buffer[j * SCROLL_X_DIM + i] = 3;
        }
    }
}

void draw_textmode_terminal() {
    int cur_plane, cur_i,i,j;
    for(j = 0; j < SCROLL_Y_DIM; j++){
        for(i = 0; i < SCROLL_X_DIM; i++){
            cur_plane = i & 3;
            cur_i = i / 4;
            switch (cur_plane) {
                case 0:
                    plane0[j * (SCROLL_X_DIM / 4) + cur_i] = draw_text_buffer[j * (SCROLL_X_DIM) + i];
                    break;
                case 1:
                    plane1[j * (SCROLL_X_DIM / 4) + cur_i] = draw_text_buffer[j * (SCROLL_X_DIM) + i];
                    break;
                case 2:
                    plane2[j * (SCROLL_X_DIM / 4) + cur_i] = draw_text_buffer[j * (SCROLL_X_DIM) + i];
                    break;
                case 3:
                    plane3[j * (SCROLL_X_DIM / 4) + cur_i] = draw_text_buffer[j * (SCROLL_X_DIM) + i];
                    break;
            }
        }
    }
}

void update_four_planes(){
    uint32_t i,j,cur_plane,cur_i;
    for(j = 0; j < SCROLL_Y_DIM; j++){
        for(i = 0; i < SCROLL_X_DIM; i++){
            cur_plane = i & 3;
            cur_i = i / 4;
            switch (cur_plane) {
                case 0:
                    plane0[j * (SCROLL_X_DIM / 4) + cur_i] = main_buffer[j * (SCROLL_X_DIM) + i];
                    break;
                case 1:
                    plane1[j * (SCROLL_X_DIM / 4) + cur_i] = main_buffer[j * (SCROLL_X_DIM) + i];
                    break;
                case 2:
                    plane2[j * (SCROLL_X_DIM / 4) + cur_i] = main_buffer[j * (SCROLL_X_DIM) + i];
                    break;
                case 3:
                    plane3[j * (SCROLL_X_DIM / 4) + cur_i] = main_buffer[j * (SCROLL_X_DIM) + i];
                    break;

            }
        }
    }
}

void show_status_bar() {


    SET_WRITE_MASK(1 << (0 + 8));
    copy_image(status_plane0, 0);
    SET_WRITE_MASK(1<<  (1 + 8));
    copy_image(status_plane1, 0);
    SET_WRITE_MASK(1<<  (2 + 8));
    copy_image(status_plane2, 0);
    SET_WRITE_MASK(1<<  (3 + 8));
    copy_image(status_plane3, 0);

    /*
     * Change the VGA registers to point the top left of the screen
     * to the video memory that we just filled.
     */
    OUTW(0x03D4, (target_img & 0xFF00) | 0x0C);
    OUTW(0x03D4, ((target_img & 0x00FF) << 8) | 0x0D);
}

