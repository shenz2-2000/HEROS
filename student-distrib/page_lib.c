//
// Created by BOURNE on 2021/4/10.
//

#include "page_lib.h"
#include "x86_desc.h"
#include "file_sys.h"


void flush_tlb() {
    asm volatile("         \n\
    movl %%cr3, %%eax      \n\
    movl %%eax, %%cr3      \n\
    "
    :
    :
    :"cc", "memory", "eax");

}

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

uint32_t get_eip(dentry_t* task_dentry_ptr){

    uint32_t return_val;

    // NULL pointer
    if(task_dentry_ptr == NULL) return -1;

    // read fail
    if( (read_data(task_dentry_ptr->inode_idx, FIRST_INSTRUCTION, (uint8_t*) &return_val, 4)) != 4) return -1;

    return return_val;

}


int load_private_code(dentry_t * task_dentry_ptr){

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












