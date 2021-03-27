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

int32_t file_sys_init(module_t *f_sys_mod) {
    // local var
    // module_t f_sys_mod;
    // f_sys_mod = *f_sys_img;
    int i;

    // init the block pointers
    bblock_ptr = (boot_block_t*)(f_sys_mod->mod_start);
    inodes_arr = ((inode_block_t*)(f_sys_mod->mod_start)) + 1;
    dblocks_arr = ((boot_block_t*)(f_sys_mod->mod_start)) + bblock_ptr->n_inodes + 1;

    // init the pcb_arr
    for (i = 2; i < N_PCB_LIMIT; i++) pcb_arr[i].flags = 0;

    // init the file operations
    file_op.open = file_open;
    file_op.close = file_close;
    file_op.read = file_read;
    file_op.write = file_write;

    return 0;
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

// generate a available fd
int32_t generate_fd() {
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

int32_t file_open(const uint8_t *f_name) {
    // target dentry
    dentry_t dentry;
    int32_t fd;
    if (read_dentry_by_name(f_name, &dentry) == -1) {
        printf("WARNING [FILE]: cannot OPEN file with name: %s\n", f_name);
        return -1;
    }

    // obtain an available fd number
    fd = generate_fd();

    // populate the block
    pcb_arr[fd].f_op = &file_op;
    pcb_arr[fd].inode_idx = dentry.inode_idx;
    pcb_arr[fd].f_pos = 0;  // the global cursor is at the beginning
    // no need to fill flags

    return fd;
}

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

int32_t file_write(int32_t fd, const void *buf, int32_t bufsize) {
    int32_t temp;
    temp = fd + bufsize;    // avoid warning
    (void) fd;

    printf("ERROR [FILE]: cannot WRITE the read-only file\n");
    return -1;
}
