//
// Created by BOURNE on 2021/4/10.
//

#include "page_lib.h"
#include "x86_desc.h"
#include "file_sys.h"
#include "lib.h"
#include "process.h"
#include "terminal.h"

// Page control structure
uint32_t n_page_in_use = 0;    // number of page in use
uint32_t page_status[N_PCB_LIMIT];  // record the status of all pages (totally 6 pages)

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
    int cur_pid;
    dentry_t task_dentry;

    // potential initialization
    if(n_page_in_use == 0){
        for(i = 0; i < N_PCB_LIMIT; i++) {
            page_status[i] = PAGE_FREE;
        }
    }

    // sanity check
    if(eip == NULL || task_file_name == NULL) {
        printf("ERROR in set_page_for_task: task_file_name or eip NULL pointer\n");
        return -1;
    } 

    // get task_dentry with sanity check
    if (read_dentry_by_name(task_file_name, &task_dentry) < 0) return -1;

    // check executable
    if(!executable_check(&task_dentry)) return -1;

    // get a page id with sanity check
    cur_pid = get_new_page_id();
    if(cur_pid < 0) return -1;

    // get new eip pointer with sanity check
    i = get_eip(&task_dentry);
    if(i == -1) return -1;
    *eip = i;

    // set the page and load code from file
    set_private_page(cur_pid);
    load_private_code(&task_dentry);

    // finally we update page_status
    // Warning: can only update it when everything is working properly!
    n_page_in_use ++;
    page_status[cur_pid] = PAGE_IN_USE;
    
    return cur_pid;
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

    *(page_directory + PRIVATE_PAGE_VA) = cur_entry;

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
    if(task_dentry_ptr == NULL) {
        printf("ERROR in get_eip: task_dentry_ptr NULL pointer\n");
        return -1;
    } 

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

    if(task_dentry_ptr == NULL) {
        printf("ERROR in get_eip: load_private_code NULL pointer\n");
        return -1;
    } 

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

    if(task_dentry_ptr == NULL) {
        printf("ERROR in get_eip: executable_check NULL pointer\n");
        return -1;
    } 

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
    for(i = 0; i < N_PCB_LIMIT; i++){
        if(page_status[i] == PAGE_FREE){
            return i;
        }
    }

    return -1;
}

/* restore_paging
 * Description: restore parent paging, and remove the pid (when halting the proc)
 * Inputs: child_id - page id of the current process
 *         parent_id - page id of its parent id
 * Return Value: -1 on failure and 0 on success
 * Side effect: None
 * */
int restore_paging(const int child_id, const int parent_id) {

    // Check arguments
    if ( (child_id >= N_PCB_LIMIT) || (page_status[child_id] == PAGE_FREE) ) {
        return -1;
    }
    set_private_page(parent_id);    // restore parent paging
    page_status[child_id] = PAGE_FREE;
    n_page_in_use--;
    return 0;
}

/* delete_paging
 * Description: delete a page, do not restore new page(parent)
 * Inputs: child_id - page id of the current process
 * Return Value: -1 on failure and 0 on success
 * Side effect: None*/

int delete_paging(const int child_id) {

    // Check arguments
    if ( (child_id >= N_PCB_LIMIT) || (page_status[child_id] == PAGE_FREE) ) {
        return -1;
    }

    page_status[child_id] = PAGE_FREE;
    n_page_in_use--;
    return 0;
}

/* set_paging
 * Description: set a page
 * Inputs: child_id - page id of the current process
 * Return Value: -1 on failure and 0 on success
 * Side effect: None*/

int set_paging(const int child_id){

    // Check arguments
    if ( (child_id >= N_PCB_LIMIT) || (page_status[child_id] == PAGE_FREE) ) {
        return -1;
    }

    set_private_page(child_id);
    return 0;
}

/* check_flag
 * Description: check the 1-bit flag is valid or not
 * Inputs: flag - the input flag
 * Return Value: 1 if valid, 0 if not
 * Side effect: None
 * */
int check_flag(uint32_t flag) {
    if (flag == 0 || flag == 1) return 1;
    return 0;
}

/* PTE_set
 * Description: set the target pte entry to be the address of a page
 * Inputs: pte - the address of target pte entry
 *         page_addr - the address of the page
 * Return Value: 0 for success, -1 for fail
 * Side effect: None
 * */
int PTE_set(PTE* pte, uint32_t page_addr, uint32_t US, uint32_t RW, uint32_t P) {
    if (pte == NULL || page_addr == NULL) {
        printf("ERROR in PTE_set: NULL input");
        return -1;
    }
    // align check
    if (page_addr & PAGE_4KB_ALIGN_CHECK) {
        return -1;
    }
    if ((!check_flag(US)) || (!check_flag(RW)) || (!check_flag(P))) {
        return -1;
    }

    pte->P = P;
    pte->US = US;
    pte->RW = RW;
    pte->Base_address = page_addr >> 12;
    
    return 0;
}

/* PDE_set
 * Description: set the target pde entry to be the address of a page table
 * Inputs: pte - the address of target pde entry
 *         page_addr - the address of the page table
 * Return Value: 0 for success, -1 for fail
 * Side effect: None
 * */
int PDE_4K_set(PDE *pde, uint32_t pt_addr, uint32_t US, uint32_t RW, uint32_t P) {
    if (pde == NULL || pt_addr == NULL) {
        printf("ERROR in PDE_set: NULL input");
        return -1;
    }
    // align check
    if (pt_addr & PAGE_4KB_ALIGN_CHECK) {
        return -1;
    }
    if ((!check_flag(US)) || (!check_flag(RW)) || (!check_flag(P))) {
        return -1;
    }

    pde->val = 0;
    pde->P = P;
    pde->US = US;
    pde->RW = RW;
    // parse for 4-kb case
    pde->Base_address = pt_addr >> 22;
    pde->reserved = (pt_addr << 10) >> (10 + 13);
    pde->PAT = (pt_addr << 19) >> (19 + 12);

    return 0;
}



int PDE_4M_set(PDE* pde, uint32_t page_addr, uint32_t US, uint32_t RW, uint32_t P) {
    if (pde == NULL || page_addr == NULL) {
        printf("ERROR in PDE_4M_set: NULL input");
        return -1;
    }
    // align check
    if (page_addr & PAGE_4MB_ALIGN_CHECK) {
        return -1;
    }
    if ((!check_flag(US)) || (!check_flag(RW)) || (!check_flag(P))) {
        return -1;
    }

    pde->val = 0;
    pde->G = 1;
    pde->P = P;
    pde->US = US;
    pde->RW = RW;
    pde->PS = 1;
    pde->Base_address = page_addr >> 22;

    return 0;
}
