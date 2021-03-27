#include "file_sys.h"
#include "multiboot.h"
#include "lib.h"

/* global var */
static boot_block_t *bblock_ptr;
static inode_block_t *inodes_arr;
static data_block_t *dblocks_arr;

int32_t file_sys_init(module_t *f_sys_mod) {
    // local var
    // module_t f_sys_mod;
    // f_sys_mod = *f_sys_img;

    // init the block pointers
    bblock_ptr = (boot_block_t*)(f_sys_mod->mod_start);
    inodes_arr = ((inode_block_t*)(f_sys_mod->mod_start)) + 1;
    dblocks_arr = ((boot_block_t*)(f_sys_mod->mod_start)) + bblock_ptr->n_inodes + 1;

    // TODO: init the operations
}

int32_t read_dentry_by_name (const uint8_t *fname, dentry_t *dentry) {
    int i;
    int fname_len;

    // bad input check
    fname_len = strlen((int8_t*)fname);
    if ((fname_len > F_NAME_LIMIT) || (fname_len == 0)) return -1;

    // iterate all the directory entries
    for (i = 0; i < bblock_ptr->n_dentries; i++) {
        if (strncmp((int8_t*)fname, 
                    (int8_t*)bblock_ptr->dentries[i].f_name, 
                    F_NAME_LIMIT) == 0) {   // 0 means they are the same
            *dentry = bblock_ptr->dentries[i];
            return 0;
        }
    }
    
    return -1;
}

int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    int i;

    // bad input check
    if ((index >= N_DENTRY_LIMIT) || (index >= bblock_ptr->n_dentries)) return -1;

    *dentry = bblock_ptr->dentries[index];
    return 0;
}

int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t length) {
    uint32_t dblock_vir_idx;
    uint32_t dblock_phy_idx;
    uint32_t cursor;    // the relative position in one block
    uint32_t pos;       // the global position within the file

    uint32_t bytes_cnt;
    uint32_t f_length = inodes_arr[inode].length_in_B;
    
    // bad input check
    if (inode >= bblock_ptr->n_inodes) return -1;

    pos = offset;
    dblock_vir_idx = offset / BLOCK_SIZE_IN_B;
    dblock_phy_idx = inodes_arr[inode].data_block_idx[dblock_vir_idx];

    if (dblock_phy_idx >= bblock_ptr->n_dblocks) return -1;

    cursor = offset - dblock_vir_idx * BLOCK_SIZE_IN_B;

    for (bytes_cnt = 0; bytes_cnt < length; bytes_cnt++) {
        if (pos >= f_length) break;     // at the end of the file

        buf[bytes_cnt] = dblocks_arr[dblock_phy_idx].content[cursor];

        pos++;
        cursor++;
        // reach the end of the block
        if (cursor >= BLOCK_SIZE_IN_B) {
            cursor = 0;
            dblock_vir_idx++;
            dblock_phy_idx = inodes_arr[inode].data_block_idx[dblock_vir_idx];
            // if the next block is not valid, meaning the file length is reached
            if (dblock_phy_idx >= bblock_ptr->n_dblocks) break;
        }
    }

    return bytes_cnt;
}
