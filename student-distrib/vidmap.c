#include "vidmap.h"
#include "page_lib.h"
#include "lib.h"
//#include "types.h"
//#include "x86_desc.h"

/* global var */
// terminal
int32_t terminal_status[MAX_TERMINAL];
terminal_struct_t *terminal_showing;     // current showing terminal
terminal_struct_t *terminal_running;     // the terminal that is occupied by a running process

terminal_struct_t* get_terminal_running() {
    return terminal_running;
}
void set_terminal_running(terminal_struct_t *terminal) {
    terminal_running = terminal;
}

terminal_struct_t* get_terminal_showing() {
    return terminal_showing;
}
void set_terminal_showing(terminal_struct_t *terminal) {
    terminal_showing = terminal;
}

// backup buffer
PTE k_bb_pt_0[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE k_bb_pt_1[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE k_bb_pt_2[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE* k_bb_pt_list[MAX_TERMINAL] = {k_bb_pt_0, k_bb_pt_1, k_bb_pt_2};

PTE u_bb_pt_0[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE u_bb_pt_1[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE u_bb_pt_2[PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
PTE* u_bb_pt_list[MAX_TERMINAL] = {u_bb_pt_0, u_bb_pt_1, u_bb_pt_2};

//PTE k_bb_pt_list[MAX_TERMINAL][PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));
//PTE u_bb_pt_list[MAX_TERMINAL][PAGE_TABLE_SIZE] __attribute__ ((aligned (ALIGN_4K)));

//page_table_struct k_bb_pt_list[MAX_TERMINAL];
//page_table_struct u_bb_pt_list[MAX_TERMINAL];

extern terminal_struct_t null_terminal;

/* vidmap_init
 * Description: initialize the video memory map
 * Inputs: None
 * Return Value: None
 * Side effect: initialize the video memory map
 * */
void vidmap_init() {
    int i, j;

    terminal_showing = &null_terminal;
    terminal_running = &null_terminal;

    for (i = 0; i < MAX_TERMINAL; i++) {
        terminal_status[i] = TERMINAL_OFF;

        /* set the backup buffers for kernel and user vid map */

        // firstly clear the back up buffers
        for (j = 0; j < BITS_4K; j++) {
            k_bb_pt_list[i][j].val = 0;
            u_bb_pt_list[i][j].val = 0;
        }

        // map the 0xB8000 of user and kernel backup buffers to backup area
        // which is 0xB9000, BA000, BB000
        PTE_set(&(k_bb_pt_list[i][VM_PTE]), VM_INDEX + (i+1) * BITS_4K, 1, 1, 1);
        PTE_set(&(u_bb_pt_list[i][VM_PTE]), VM_INDEX + (i+1) * BITS_4K, 0, 1, 1);

        // map to the kernel page itself
        page_table0[VM_PTE + i + 1] = k_bb_pt_list[i][VM_PTE];

        // turn on all the terminals
        terminal_status[i] = TERMINAL_ON; // the terminals are definitely on
    }

    flush_tlb();
}

/* set_video_memory
 * Description: setup page directory for user manipulation
 * Inputs: None
 * Return Value: None
 * Side effect: None
 * */
void set_video_memory(terminal_struct_t *terminal){

    // the page directory entry we need to setup
    PDE *u_vm_pde = &(page_directory[U_VM_PDE]);

    if (terminal == &null_terminal) {
        // turn off the user vidmap
        u_vm_pde->val = 0;
    } else {
        if (terminal == terminal_showing) {
            // make the memory map show
            PDE_4K_set(u_vm_pde, (uint32_t) (&video_page_table0), 1, 1, 1);    // user level
        } else {
            // map the video mem to backup buffers
            PDE_4K_set(u_vm_pde, (uint32_t) (&u_bb_pt_list[terminal->id]), 1, 1, 1);
        }
    }

    flush_tlb();
}

/* clear_video_memory
 * Description: clear page directory for user manipulation
 * Inputs: None
 * Return Value: None
 * Side effect: None
 * */
// FIXME: ??? what's this for??
void clear_video_memory(){

    // the page directory entry we need to setup
    PDE video_entry;

    video_entry.val = 0;

    // fill in the directory entry #33
    page_directory[PRIVATE_PAGE_VA + 1] = video_entry;

    flush_tlb();
}

