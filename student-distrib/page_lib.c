//
// Created by BOURNE on 2021/4/10.
//

#include "page_lib.h"
#include "x86_desc.h"
#include "file_sys.h"

// Page control structure
static uint32_t page_in_use = 0;
static uint32_t page_id_center[MAX_PAGE] = {0};

/* flush_tlb
 * Description: update the cursor position
 * Inputs: none
 * Return Value: none
 * Function: reload value of cr3
 * Side effect: change value of eax*/

void flush_tlb() {
    asm volatile("         \n\
    movl %%cr3, %%eax      \n\
    movl %%eax, %%cr3      \n\
    "
    :
    :
    :"cc", "memory", "eax");
}


/* set_page_for_task
 * Description: do the background setup for the new task
 * Inputs: task_file_name -- file name of new task
 *         eip -- pointer for new eip we need to fill
 * Return Value: -1 -- failure
 *               page_id -- success
 * Function: set up page directory and load the code for the new task (with sanity check)
 * Side effect: change the value of Page Directory*/

int set_page_for_task(uint8_t* task_file_name, uint32_t* eip){

    int i;
    int cur_page_id;
    dentry_t task_dentry;

    // sanity check
    if(eip == NULL || task_file_name == NULL) return -1;

    // potential initialization
    if(page_in_use == 0){
        for(i = 0; i < MAX_PAGE; i++)
            page_id_center[i] = 0;
    }

    // get task_dentry with sanity check
    if (read_dentry_by_name(task_file_name, &task_dentry) < 0) return -1;

    // check executable
    i = executable_check(&task_dentry);
    if(i == 0) return -1;

    // get a page id with sanity check
    cur_page_id = get_new_page_id();
    if(cur_page_id < 0) return -1;

    // get new eip pointer with sanity check
    i = get_eip(&task_dentry);
    if(i == -1) return -1;
    *eip = i;

    // set the page and load code from file
    set_private_page(cur_page_id);
    load_private_code(&task_dentry);

    // finally we update page_id_center
    // Warning: can only update it when everything is working properly!
    page_in_use ++;
    page_id_center[cur_page_id] = 1;

    return cur_page_id;
}


/* set_private_page
 * Description: set page directory entry for the new task
 * Inputs: pid -- page id (allocate in set_page_for_task)
 * Return Value: none
 * Side effect: change value of PD and flush the TLB*/

void set_private_page(int32_t pid){

    // initialization of current pde
    uint32_t cur_PDE = ((pid + 2) << 22) | PDE_MASK ;

    // set the PDE entry
    PDE cur_entry;
    cur_entry.P = cur_PDE & 0x00000001;
    cur_entry.RW = (cur_PDE >> 1) & 0x00000001;
    cur_entry.US = (cur_PDE >> 2)  & 0x00000001;
    cur_entry.PWT = (cur_PDE >> 3)  & 0x00000001;
    cur_entry.PCD = (cur_PDE >> 4)  & 0x00000001;
    cur_entry.A = (cur_PDE >> 5)  & 0x00000001;
    cur_entry.D = (cur_PDE >> 6)  & 0x00000001;
    cur_entry.PS = (cur_PDE >> 7)  & 0x00000001;
    cur_entry.G = (cur_PDE >> 8)  & 0x00000001;
    cur_entry.AVAIAL = (cur_PDE >> 9)  & 0x00000007;
    cur_entry.PAT = (cur_PDE >> 12) & 0x00000001;
    cur_entry.reserved = (cur_PDE >> 13) & 0x000001FF;
    cur_entry.Base_address = (cur_PDE >> 22);

    page_directory[PRIVATE_PAGE_VA] = cur_entry;

      // flush the CR3 register
      flush_tlb();
}

/* get_eip
 * Description: get a new eip from task file
 * Inputs: pid -- page id (allocate in set_page_for_task)
 * Return Value: -1 -- failure
 *               return_val -- new eip
 * Side effect: read file*/

int get_eip(dentry_t* task_dentry_ptr){

    int return_val;

    // NULL pointer
    if(task_dentry_ptr == NULL) return -1;

    // read fail
    if( (read_data(task_dentry_ptr->inode_idx, FIRST_INSTRUCTION, (uint8_t*) &return_val, 4)) != 4) return -1;

    return return_val;

}

/* load_private_code
 * Description: load code to the prescribed virtual memory
 * Inputs: task_dentry_ptr -- ptr to task dentry
 * Return Value: -1 -- failure
 *               0 -- success
 * Side effect: read file*/

int load_private_code(dentry_t * task_dentry_ptr){

    if(task_dentry_ptr == NULL) return -1;

    // variable declaration
    int i;
    uint32_t inode_idx = task_dentry_ptr->inode_idx;
    uint32_t size = get_file_length(task_dentry_ptr);

    // set target base address
    uint8_t temp;
    uint32_t one_time = 1;
    uint8_t * target_base_address = (uint8_t* ) CODE_BASE_VA;

    for(i=0; i < size; i++){
        // read from file
        read_data(inode_idx,i,&temp,one_time);
        // copy to code image region
        target_base_address[i] = temp;
    }

    return 0;
}

/* executable_check
 * Description: check whether the file is executable
 * Inputs: task_dentry_ptr -- ptr to task dentry
 * Return Value: 0 -- failure
 *               1 -- success
 * Side effect: read file*/

int executable_check(dentry_t* task_dentry_ptr){
    uint8_t temp[ELF_LENGTH];
    read_data(task_dentry_ptr->inode_idx,0,temp,ELF_LENGTH);

    // four bad cases
    if(temp[0] != 0x7F) return 0;
    if(temp[1] != 'E')  return 0;
    if(temp[2] != 'L')  return 0;
    if(temp[3] != 'F')  return 0;

    return 1;

}

/* get_new_page_id
 * Description: get a free page id
 * Inputs: none
 * Return Value: -1 -- failure
 *               i -- page id
 * Side effect: None*/

int get_new_page_id(){
    int i;
    for(i = 0; i < MAX_PAGE; i++){
        if(page_id_center[i] == 0){
            return i;
        }
    }

    return -1;
}












