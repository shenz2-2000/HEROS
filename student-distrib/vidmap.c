#include "vidmap.h"
#include "page_lib.h"
#include "lib.h"
#include "terminal.h"

/* global var */
// terminal
static int32_t terminal_status[MAX_TERMINAL];


void vidmap_init() {
    int i;

    for (i = 0; i < MAX_TERMINAL; i++) {
        terminal_status[i] = TERMINAL_OFF;
    }

    // set the backup buffers for kernel and user vid map
    for (i = 0; i < MAX_TERMINAL; i++) {
        
    }
}

/* set_video_memory
 * Description: setup page directory for user manipulation
 * Inputs: None
 * Return Value: None
 * Side effect: None*/

void set_video_memory(){

    // the page directory entry we need to setup
    PDE video_entry;
    uint32_t pt_addr = (uint32_t) (&video_page_table0);     // TODO: set pde and pte as param!

    // setup the base address (20 bit)
    video_entry.Base_address = pt_addr >> 22;
    video_entry.reserved = (pt_addr << 10) >> (10 + 13);
    video_entry.PAT = (pt_addr << 19) >> (19 + 12);

    // setup the important parameter
    video_entry.P = 1;
    video_entry.RW = 1;
    video_entry.US = 1;

    // setup the default parameter
    video_entry.PWT = 0;
    video_entry.PCD = 0;
    video_entry.A = 0;
    video_entry.D = 0;
    video_entry.PS = 0;
    video_entry.G = 0;
    video_entry.AVAIAL = 0;

    // fill in the directory entry #33
    page_directory[PRIVATE_PAGE_VA + 1] = video_entry;

    flush_tlb();
}

/* clear_video_memory
 * Description: clear page directory for user manipulation
 * Inputs: None
 * Return Value: None
 * Side effect: None*/

void clear_video_memory(){

    // the page directory entry we need to setup
    PDE video_entry;

    // setup the base address (20 bit)
    video_entry.Base_address = 0;
    video_entry.reserved = 0;
    video_entry.PAT = 0;

    // setup the important parameter
    video_entry.P = 0;
    video_entry.RW = 0;
    video_entry.US = 0;

    // setup the default parameter
    video_entry.PWT = 0;
    video_entry.PCD = 0;
    video_entry.A = 0;
    video_entry.D = 0;
    video_entry.PS = 0;
    video_entry.G = 0;
    video_entry.AVAIAL = 0;

    // fill in the directory entry #33
    page_directory[PRIVATE_PAGE_VA + 1] = video_entry;

    flush_tlb();

}
