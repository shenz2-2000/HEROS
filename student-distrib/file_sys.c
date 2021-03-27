#include "file_sys.h"
#include "multiboot.h"
#include "lib.h"

/* global var */
// block discription
static boot_block_t *bblock_ptr;
static inode_block_t *inodes_arr;
static data_block_t *dblocks_arr;

// file array
static int32_t n_opend_files = 2;   // the first and second pcb are for stdin and stdout
static pcb_t pcb_arr[N_PCB_LIMIT];

// op table
static file_operations_t file_op;
static file_operations_t terminal_op;
static file_operations_t rtc_op;
static file_operations_t dir_op;

/**
 * file_sys_init
 * Description: Initialize the file system
 * Input: f_sys_mod - multiboot module of file system image.
 * Output: 0 if success.
 * Side effect: Initialize the file system
 */
int32_t file_sys_init(module_t *f_sys_mod) {
    // local var
    // module_t f_sys_mod;
    // f_sys_mod = *f_sys_img;
    int i;

    // init the block pointers
    bblock_ptr = (boot_block_t*)(f_sys_mod->mod_start);
    inodes_arr = ((inode_block_t*)(f_sys_mod->mod_start)) + 1;
    dblocks_arr = ((boot_block_t*)(f_sys_mod->mod_start)) + bblock_ptr->n_inodes + 1;

    // init the file operations
    file_op.open = file_open;
    file_op.close = file_close;
    file_op.read = file_read;
    file_op.write = file_write;

    pcb_arr[0].f_op = &terminal_op;
    pcb_arr[0].flags = 1;
    pcb_arr[0].f_pos = 0;
    pcb_arr[0].inode_idx = 0;
    pcb_arr[1].f_op = &terminal_op;
    pcb_arr[1].flags = 1;
    pcb_arr[1].f_pos = 0;   // FIXME: what should be the position???
    pcb_arr[1].inode_idx = 1;

    // init the pcb_arr
    for (i = 2; i < N_PCB_LIMIT; i++) pcb_arr[i].flags = 0;

    return 0;
}

/**
 * read_dentry_by_name
 * Description: read the dentry by the file name
 * Input: fname - the file name
          dentry - the dentry struct
 * Output: 0 if success, -1 if not.
 * Side effect: the dentry pointer will be filled with output
 */
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

/**
 * read_dentry_by_index
 * Description: read the dentry by the index
 * Input: fname - the dentry index
          dentry - the dentry struct
 * Output: 0 if success, -1 if not.
 * Side effect: the dentry pointer will be filled with output
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry) {
    int i;

    // bad input check
    if ((index >= N_DENTRY_LIMIT) || (index >= bblock_ptr->n_dentries)) return -1;

    *dentry = bblock_ptr->dentries[index];
    return 0;
}

/**
 * read_data
 * Description: read content of a file
 * Input: inode - the inode struct
          offset - the offset of reading position
          buf - the read buffer
          bufsize - the size of the buffer
 * Output: the number of bytes read
 * Side effect: the buffer will be filled with content
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t bufsize) {
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

    for (bytes_cnt = 0; bytes_cnt < bufsize; bytes_cnt++) {
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

/**
 * allocate_fd
 * Description: generate a available fd
 * Input: None
 * Output: the allocated fd
 * Side effect: None
 */
int32_t allocate_fd() {
    int i;
    if (n_opend_files >= N_PCB_LIMIT) return -1;
    for (i = 2; i < N_PCB_LIMIT; i++) {
        if (pcb_arr[i].flags == 0) {    // this entry is available
            pcb_arr[i].flags = 1;       // this flag must be filled here (avoid race)
            break;
        }
    }
    return i;
}

/**
 * file_open
 * Description: open a file
 * Input: f_name - file name
 * Output: the allocated fd
 * Side effect: the file is in use
 */
int32_t file_open(const uint8_t *f_name) {
    // target dentry
    dentry_t dentry;
    int32_t fd;
    if (read_dentry_by_name(f_name, &dentry) == -1) {
        printf("WARNING [FILE]: cannot OPEN file with name: %s\n", f_name);
        return -1;
    }

    // obtain an available fd number
    fd = allocate_fd();

    // populate the block
    pcb_arr[fd].f_op = &file_op;
    pcb_arr[fd].inode_idx = dentry.inode_idx;
    pcb_arr[fd].f_pos = 0;  // the global cursor is at the beginning
    // no need to fill flags

    return fd;
}

/**
 * file_close
 * Description: close a file
 * Input: fd - the file descriptor
 * Output: 0 if success
 * Side effect: the file is not in use
 */
int32_t file_close(int32_t fd) {
    
    if (fd == 0) {printf("ERROR [FILE]: cannot CLOSE stdin\n"); return -1;}
    if (fd == 1) {printf("ERROR [FILE]: cannot CLOSE stdout\n"); return -1;}
    if (fd < 0 || fd > N_PCB_LIMIT) {
        printf("ERROR [FILE]: fd overflow\n");
        return -1;
    }
    if (pcb_arr[fd].flags == 0) {
        printf("WARNING [FILE]: cannot CLOSE a file that is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }

    pcb_arr[fd].flags = 0;  // empty the block
    n_opend_files--;

    return 0;
}

/**
 * file_read
 * Description: get the content of a file
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: 0 if success
 * Side effect: the buffer will be filled
 */
int32_t file_read(int32_t fd, void *buf, int32_t bufsize) {
    int32_t bytes_cnt;
    uint32_t offset = pcb_arr[fd].f_pos;

    // read the data
    bytes_cnt = read_data(pcb_arr[fd].inode_idx, offset, buf, bufsize);
    if (bytes_cnt == -1) return -1;

    // update position
    pcb_arr[fd].f_pos += bytes_cnt;

    return bytes_cnt;
}

/**
 * file_write
 * Description: write to a file (NOTE: read-only file system!)
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: -1: fail
 * Side effect: an error will be promped
 */
int32_t file_write(int32_t fd, const void *buf, int32_t bufsize) {
    int32_t temp;
    temp = fd + bufsize;    // avoid warning
    (void) fd;

    printf("ERROR [FILE]: cannot WRITE the read-only file\n");
    return -1;
}
