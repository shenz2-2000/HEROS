//
// Created by BOURNE on 2021/4/10.
//

#ifndef FAKE_PAGE_LIB_H
#define FAKE_PAGE_LIB_H

#include "x86_desc.h"
#include "file_sys.h"
#include "terminal.h"


#define PDE_IDX         128
#define PRIVATE_PAGE_VA 32
#define PDE_MASK        0x00000087

#define CODE_BASE_VA    0x08048000
#define ELF_LENGTH      4
#define FIRST_INSTRUCTION 24

#define USER_VA_START 0x08000000
#define USER_VA_END   0x08400000

// task paging
#define PAGE_IN_USE 1
#define PAGE_FREE   0

// alignment check
#define PAGE_4KB_ALIGN_CHECK    0x0FFF
#define PAGE_4MB_ALIGN_CHECK    0x003FFFFF

// tool functions
extern void flush_tlb();
extern int get_eip(dentry_t * task_dentry_ptr);
extern void set_private_page(int32_t pid);
extern int load_private_code(dentry_t * task_dentry_ptr);
extern int executable_check(dentry_t * task_dentry_ptr);
extern int set_page_for_task(uint8_t* task_file_name, uint32_t* eip);
extern int get_new_page_id();
extern int delete_paging(const int child_id);

int restore_paging(const int child_id, const int parent_id);

int check_flag(uint32_t flag);

int PTE_set(PTE* pte, uint32_t page_addr, uint32_t US, uint32_t RW, uint32_t P);

int PDE_4K_set(PDE *pde, uint32_t pt_addr, uint32_t US, uint32_t RW, uint32_t P);

int PDE_4M_set(PDE* pde, uint32_t page_addr, uint32_t US, uint32_t RW, uint32_t P);

#endif //FAKE_PAGE_LIB_H
