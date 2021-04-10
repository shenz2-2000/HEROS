#include "file_sys.h"
#include "multiboot.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"
/* global var */
// block discription
static boot_block_t *bblock_ptr;    // boot block
static inode_block_t *inodes_arr;   // inode block
static data_block_t *dblocks_arr;   // data block

// file array
static int32_t n_opend_files = 2;   // the first and second file struct are for stdin and stdout
static file_struct_t file_arr[N_FILE_LIMIT];  // the file struct array

// op table
static file_operations_t file_op;       // the op table for regular file
static file_operations_t terminal_op;   // the op table for terminal
static file_operations_t rtc_op;        // the op table for rtc file
static file_operations_t dir_op;        // thr op table for directory

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
    dblocks_arr = ((data_block_t*)(f_sys_mod->mod_start)) + bblock_ptr->n_inodes + 1;

    // init the file operations
    file_op.open = file_open;
    file_op.close = file_close;
    file_op.read = file_read;
    file_op.write = file_write;

    terminal_op.open =  terminal_open;
    terminal_op.read =  terminal_read;
    terminal_op.write =  terminal_write;
    terminal_op.close =  terminal_close;

    dir_op.open =  dir_open;
    dir_op.read =  dir_read;
    dir_op.write =  dir_write;
    dir_op.close =  dir_close;

    rtc_op.open =  file_rtc_open;
    rtc_op.read =  file_rtc_read;
    rtc_op.write =  file_rtc_write;
    rtc_op.close =  file_rtc_close;

    // init the file_arr
    file_arr[0].f_op = &terminal_op;
    file_arr[0].flags = OCCUPIED;
    file_arr[0].f_pos = 0;
    file_arr[0].inode_idx = 0;
    file_arr[1].f_op = &terminal_op;
    file_arr[1].flags = OCCUPIED;
    file_arr[1].f_pos = 1;   // FIXME: what should be the position???
    file_arr[1].inode_idx = 1;

    for (i = 2; i < N_FILE_LIMIT; i++) file_arr[i].flags = AVAILABLE;

    return 0;
}

//-------------------------------------READ ROUTINE-----------------------------------

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
    printf("in read_dentry: &fname: %d\n", fname);
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
    uint32_t dblock_vir_idx;    // the data block index in the block array in inode
    uint32_t dblock_phy_idx;    // the data block index as absolute block numbers
    uint32_t cursor;            // the relative position in one block
    uint32_t pos;               // the global position within the file

    uint32_t bytes_cnt = 0;     // how many bytes are read
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

            // if a bad data block number is found within the ﬁle bounds 
            // of the given inode, the function should also return -1.
            if (dblock_phy_idx >= bblock_ptr->n_dblocks) return -1;
        }
    }

    return bytes_cnt;
}

/**
 * read_dentry_by_inode
 * Description: read the dentry by the inode index
 * Input: fname - the dentry index
          dentry - the dentry struct
 * Output: 0 if success, -1 if not.
 * Side effect: the dentry pointer will be filled with output
 */
int32_t read_dentry_by_inode(uint32_t inode, dentry_t *dentry) {
    int i;
    // iterate all the directory entries
    for (i = 0; i < bblock_ptr->n_dentries; i++) {
        if (inode == bblock_ptr->dentries[i].inode_idx) {   // 0 means they are the same
            *dentry = bblock_ptr->dentries[i];
            return 0;
        }
    }
    return -1;
}

/**
 * get_file_length
 * Description: get the length of file
 * Input: dentry - the dentry struct
 * Output: 0 or positive length if success, -1 if not.
 * Side effect: None
 */
int32_t get_file_length(dentry_t *dentry) {
    if (dentry == NULL) {
        printf("ERROR in get_file_length: dentry NULL pointer");
        return -1;
    }
    
    return inodes_arr[dentry->inode_idx].length_in_B;
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
    if (n_opend_files >= N_FILE_LIMIT) return -1;
    for (i = 2; i < N_FILE_LIMIT; i++) {
        if (file_arr[i].flags == AVAILABLE) {    // this entry is available
            file_arr[i].flags = OCCUPIED;       // this flag must be filled here (avoid race)
            break;
        }
    }
    return i;
}

//-------------------------------------FILE OPERATIONS-----------------------------------

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
        printf("WARNING [FILE]: cannot OPEN file with name: [%s]\n", f_name);
        return -1;
    }

    // obtain an available fd number
    fd = allocate_fd();
    // printf("In file_open: Now the fd = %d\n", fd);

    // populate the block
    file_arr[fd].f_op = &file_op;
    file_arr[fd].inode_idx = dentry.inode_idx;
    file_arr[fd].f_pos = 0;  // the global cursor is at the beginning
    // no need to fill flags (it's filled in fd allocation)

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
    uint32_t offset = file_arr[fd].f_pos;

    // read the data
    bytes_cnt = read_data(file_arr[fd].inode_idx, offset, buf, bufsize);
    if (bytes_cnt == -1) return -1;

    // update position
    file_arr[fd].f_pos += bytes_cnt;

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
    // avoid warning
    int32_t temp;
    temp = fd + bufsize;    
    (void) fd;

    // the fs is read-only
    printf("ERROR [FILE]: cannot WRITE the read-only file\n");
    return -1;
}


//-------------------------------------DIRECTORY OPERATIONS-----------------------------------

/**
 * dir_write
 * Description: no need to do anything
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: -1 fail
 * Side effect: none
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t bufsize){
    // just to avoid warning
    fd++;
    bufsize++;
    (void) buf;
    // do not need to do anything meaningful
    printf("ERROR [FILE]: cannot WRITE directory\n");
    return -1;
}

/**
 * dir_close
 * Description: call file_close
 * Input: fd - the file descriptor
 * Output: 0 : success
 * Side effect: none
 */
int32_t dir_close(int32_t fd){
    // call file_close to reset flag
    // file_close(fd);
    return 0;
}

/**
 * dir_open
 * Description: call file_close
 * Input: f_name -- no need to use
 * Output: cur_fd -- the newly opened fd
 * Side effect: none
 */
int32_t dir_open(const uint8_t *f_name){
    // we do not need to use filename, since this is a single level file system
    int32_t cur_fd = allocate_fd();

    // initialize the file struct array entry
    file_arr[cur_fd].inode_idx = 0;
    file_arr[cur_fd].f_pos = 0;
    file_arr[cur_fd].flags = OCCUPIED;
    file_arr[cur_fd].f_op = &dir_op;

    // we could use this to read file names under the '.' directory
    return cur_fd;

}

/**
 * dir_read
 * Description: call file_close
 * Input: fd -- file descriptor
 *        buf -- the user buffer
 *        bufsize -- number of bytes user wants
 * Output: number of bytes actually copy
 * Side effect: none
 */
int32_t dir_read(int32_t fd, void *buf, int32_t bufsize){

    // cur file position to read
    int cur_pos = file_arr[fd].f_pos;
    cur_pos++;

    // check the #bytes to read
    bufsize = (bufsize > F_NAME_LIMIT) ? F_NAME_LIMIT : bufsize;

    // sanity check
    if( cur_pos > ( bblock_ptr->n_dentries - 1 ) ) return 0;

    // copy to buffer
    strncpy(buf, (int8_t *) (bblock_ptr->dentries[cur_pos].f_name), bufsize);
    file_arr[fd].f_pos = cur_pos;

    return bufsize;

}


/*************************Linkage Between RTC and RTC in file system***************/


/**
 * file_rtc_open
 * Description: Just get a file descriptor in file struct for RTC
 * Input: f_name - file name
 * Output: the allocated fd
 * Side effect: One posistion in the file struct array is occupied
 */
int32_t file_rtc_open(const uint8_t *f_name) {
    int32_t fd;
    
    if (rtc_open(NULL)!=0) return -1;
    // obtain an available fd number
    fd = allocate_fd();

    // FIXME: if there exists an opened rtc file, then we just reopen it on the same fd
    
    // populate the block
    file_arr[fd].f_op = &rtc_op;
    file_arr[fd].inode_idx = 0;
    file_arr[fd].f_pos = 0;  // the global cursor is at the beginning
    // no need to fill flags

    return fd;
}


/**
 * file_rtc_close
 * Description: close the RTC driver
 * Input: fd - the file descriptor
 * Output: 0 if success -1 if unccessful
 * Side effect: release the place in file struct array
 */
int32_t file_rtc_close(int32_t fd) {
    // if (file_close(fd)!=0 || rtc_close(fd)!=0) return -1;
    if (rtc_close(fd)!=0) return -1;
    return 0;
}


/**
 * file_rtc_read
 * Description: call rtc_read
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: 0 if success
 */
int32_t file_rtc_read(int32_t fd, void *buf, int32_t bufsize) {
    return rtc_read(fd, buf, bufsize);
}

/**
 * file_rtc_write
 * Description: call rtc_write
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: 0 if success
 */
int32_t file_rtc_write(int32_t fd, const void *buf, int32_t bufsize) {
    return rtc_write(fd, buf, bufsize);
}

//-------------------------------------SYSTEM CALL-----------------------------------

/**
 * sys_open
 * Description: system call: open a file
 * Input: f_name - file name
 * Output: the allocated fd
 * Side effect: the file is in use
 */
int32_t sys_open(const uint8_t *f_name) {
    dentry_t dentry;
    int32_t fd = -1;

    if (n_opend_files >= N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_open: cannot OPEN file [%s] because the max number of files is reached", f_name);
        return -1;
    }

    // obtain the dentry to know the file type
    if (read_dentry_by_name(f_name, &dentry) == -1) {
        printf("WARNING [SYS FILE] in sys_open: cannot OPEN file with name: [%s]\n", f_name);
        return -1;
    }

    // invoke the different open op according to file type
    switch (dentry.f_type) {
        case 0: {   // RTC file
            fd = file_rtc_open(f_name);
            break;
        }
        case 1: {   // directory file
            fd = dir_open(f_name);
            break;
        }
        case 2: {   // regulary file
            fd = file_open(f_name);
            break;
        }
    }

    n_opend_files++;

    return fd;
}

/**
 * sys_close
 * Description: system call: close a file
 * Input: fd - the file descriptor
 * Output: 0 if success
 * Side effect: the file is not in use
 */
int32_t sys_close(int32_t fd) {
    int32_t ret;

    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [FILE]: fd overflow\n");
        return -1;
    }
    if (fd == 0) {printf("ERROR [FILE]: cannot CLOSE stdin\n"); return -1;}
    if (fd == 1) {printf("ERROR [FILE]: cannot CLOSE stdout\n"); return -1;}
    if (file_arr[fd].flags == AVAILABLE) {
        printf("WARNING [FILE]: cannot CLOSE a file that is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }

    ret = file_arr[fd].f_op->close(fd);

    file_arr[fd].flags = AVAILABLE;  // empty the block
    n_opend_files--;

    return ret;
}

/**
 * sys_read
 * Description: system call: get the content of a file
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: 0 if success
 * Side effect: the buffer will be filled
 */
int32_t sys_read(int32_t fd, void *buf, int32_t bufsize) {
    // bad input checking
    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_read: fd overflow\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR [SYS FILE] in sys_read: read buffer NULL pointer");
        return -1;
    }
    if (bufsize < 0) {
        printf("ERROR [SYS FILE] in sys_read: read buffer size should be non-negative");
        return -1;
    }
    if (file_arr[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_read: cannot READ a file that is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }
    // stdout is write-only
    if (fd == 1) {
        printf("ERROR [SYS FILE] in sys_read: stdout is write-only");
        return -1;
    }

    /* for debug */
    // dentry_t dentry;
    // read_dentry_by_inode(file_arr[fd].inode_idx, &dentry);
    // printf("In file_test: Now the file type is %d\n", dentry.f_type);

    return file_arr[fd].f_op->read(fd, buf, bufsize);
}

/**
 * sys_write
 * Description: system call: write to a file (NOTE: read-only file system!)
 * Input: fd - the file descriptor
          buf - the buffer
          bufsize - the number of bytes of a buffer
 * Output: -1: fail
 * Side effect: an error will be promped
 */
int32_t sys_write(int32_t fd, const void *buf, int32_t bufsize) {
    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_write: fd overflow\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR [SYS FILE] in sys_write: write buffer NULL pointer");
        return -1;
    }
    if (bufsize < 0) {
        printf("ERROR [SYS FILE] in sys_write: write buffer size should be non-negative");
        return -1;
    }
    if (file_arr[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_write: the file is not opened. fd: %d\n", fd);
        return 0;   // not a serious error
    }
    return file_arr[fd].f_op->write(fd, buf, bufsize);
}
