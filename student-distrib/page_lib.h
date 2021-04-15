//
// Created by BOURNE on 2021/4/10.
//

#ifndef FAKE_PAGE_LIB_H
#define FAKE_PAGE_LIB_H

#include "x86_desc.h"
#include "file_sys.h"


#define PDE_IDX         128
#define PRIVATE_PAGE_VA 32
#define PDE_MASK        0x00000087
#define CODE_BASE_VA    0x08048000
#define ELF_LENGTH      4
#define FIRST_INSTRUCTION 24
#define MAX_PAGE 2
#define USER_VA_START 0x08000000
#define USER_VA_END   0x08400000
#define VM_INDEX      0xB8000


// tool functions
extern void flush_tlb();
extern int get_eip(dentry_t * task_dentry_ptr);
extern void set_private_page(int32_t pid);
extern int load_private_code(dentry_t * task_dentry_ptr);
extern int executable_check(dentry_t * task_dentry_ptr);
extern int set_page_for_task(uint8_t* task_file_name, uint32_t* eip);
extern int get_new_page_id();
extern void set_video_memory();
extern void clear_video_memory();
extern int sys_vidmap(uint8_t** screen_start);

int restore_paging(const int child_id, const int parent_id);


#endif //FAKE_PAGE_LIB_H
