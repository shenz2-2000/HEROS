#include "vidmap.h"
#include "page_lib.h"
#include "lib.h"
#include "types.h"
#include "terminal.h"
#include "x86_desc.h"

/* global var */
// terminal
static int32_t terminal_status[MAX_TERMINAL];
static terminal_struct_t *terminal_showing;     // current showing terminal
static terminal_struct_t *terminal_running;     // the terminal that is occupied by a running process

// backup buffer
static PTE k_bb_pt_list[MAX_TERMINAL][PAGE_TABLE_SIZE];
static PTE u_bb_pt_list[MAX_TERMINAL][PAGE_TABLE_SIZE];

/* vidmap_init
 * Description: initialize the video memory map
 * Inputs: None
 * Return Value: None
 * Side effect: initialize the video memory map
 * */
void vidmap_init() {
    int i, j;

    terminal_showing = NULL;
    terminal_running = NULL;

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
        terminal_status[i] = TERMINAL_ON;
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

    if (terminal == NULL) {
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

/* ------------------- terminal operation --------------------- */

/* terminal_turn_on
 * Description: turn on the terminal
 * Inputs: terminal - the target terminal struct
 * Return Value: None
 * Side effect: None
 * */
int terminal_turn_on(terminal_struct_t *terminal) {
    if (terminal == NULL) {
        printf("ERROR in terminal_turn_on(): NULL input");
        return -1;
    }
    if (terminal->id < 0 || terminal->id >= MAX_TERMINAL) {
        printf("ERROR in terminal_turn_on(): no such terminal_id: %d", terminal->id);
        return -1;
    }
    if (terminal_status[terminal->id] == TERMINAL_ON) {
        printf("WARNING in terminal_turn_on(): the terminal %d is already turned on\n", terminal->id);
    }

    terminal_status[terminal->id] = TERMINAL_ON;
    return 0;
}

/* swtich_terminal
 * Description: switch the terminal
 * Inputs: old_terminal - the old shown terminal(to be stored). this param can be NULL because the 
 *                   process can allocate a terminal from nothing
 *         new_terminal - the new terminal to be shown (this param can also be NULL)
 * Return Value: None
 * Side effect: None
 * */
int swtich_terminal(terminal_struct_t *old_terminal, terminal_struct_t *new_terminal) {
    // sanity check
    if (old_terminal == new_terminal) return 0;
    if (old_terminal != NULL) {
        if (old_terminal->id < 0 || old_terminal->id >= MAX_TERMINAL) {
            printf("ERROR in swtich_terminal(): bad input of old_terminal\n");
            return -1;
        }
        if (terminal_status[old_terminal->id] == TERMINAL_OFF) {
            printf("ERROR in swtich_terminal(): old_terminal not turned on\n");
            return -1;
        }
    }
    if (new_terminal != NULL) {
        if (new_terminal->id < 0 || new_terminal->id >= MAX_TERMINAL) {
            printf("ERROR in swtich_terminal(): bad input of new_terminal\n");
            return -1;
        }
        if (terminal_status[new_terminal->id] == TERMINAL_OFF) {
            printf("ERROR in swtich_terminal(): new_terminal not turned on\n");
            return -1;
        }
    }

    // copy the physical video memory to the backup buffer
    if (old_terminal != NULL) {
        memcpy((uint8_t *) (VM_INDEX + (old_terminal->id + 1) * BITS_4K), (uint8_t *) (VM_INDEX), BITS_4K);
    }

    // copy the backup buffer of new terminal to the video memory area
    if (new_terminal != NULL) {
        memcpy((uint8_t *) (VM_INDEX), (uint8_t *) (VM_INDEX + (old_terminal->id + 1) * BITS_4K), BITS_4K);
    }

    terminal_showing = new_terminal;
    terminal_vidmap(terminal_running);

    return 0;    
}

/* terminal_vidmap
 * Description: map the target terminal to the corresponding memory area (video mem or backup buffer)
 * Inputs: terminal - the target terminal
 * Return Value: 0 for success
 * Side effect: None
 * */
int terminal_vidmap(terminal_struct_t *terminal) {
    int ret;

    // sanity check
    if (terminal != NULL) {
        if (terminal->id < 0 || terminal->id >= MAX_TERMINAL) {
            printf("ERROR in terminal_vidmap(): bad input of terminal\n");
            return -1;
        }
        if (terminal_status[terminal->id] == TERMINAL_OFF) {
            printf("ERROR in terminal_vidmap(): terminal not turned on\n");
            return -1;
        }
    }

    if (terminal == NULL || (terminal != NULL && terminal->id == terminal_showing)) {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) &(page_table0), 0, 1, 1);
        if (ret == -1) return -1;
    } else {
        ret = PDE_4K_set(&(page_directory[0]), (uint32_t) &(k_bb_pt_list[terminal->id]), 0, 1, 1);
        if (ret == -1) return -1;
    }
    terminal_running = terminal;
    flush_tlb();
    return 0;
}
