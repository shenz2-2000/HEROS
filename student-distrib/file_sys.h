#ifndef     _FILE_SYS_H
#define     _FILE_SYS_H

#include "types.h"
#include "multiboot.h"

// consts
#define F_NAME_LIMIT    32          // in bytes

#define BLOCK_SIZE_IN_B             4096
#define STATS_ENTRY_SIZE_IN_B       64
#define DENTRY_SIZE_IN_B            64

#define N_DENTRY_LIMIT              (BLOCK_SIZE_IN_B - STATS_ENTRY_SIZE_IN_B) / DENTRY_SIZE_IN_B
#define INODE_N_DBLOCK_LIMIT        (BLOCK_SIZE_IN_B - 4) / 4   // 4: an entry in inode block takes 4 bytes

// the max number of opened files
#define N_FILE_LIMIT     8

// file structure flags
#define OCCUPIED  1
#define AVAILABLE 0

// the fd for terminal
#define STDIN   0
#define STDOUT  1

// file operation table
typedef struct file_operations {
    int32_t (*open) (const uint8_t *f_name);
    int32_t (*close) (int32_t fd);
    int32_t (*read) (int32_t fd, void *buf, int32_t bufsize);
    int32_t (*write) (int32_t fd, const void *buf, int32_t bufsize);
    int32_t (*ioctl) (int32_t fd, int32_t cmd);
} file_operations_t;

// entries in the file array
typedef struct file_struct {
    file_operations_t *f_op;    // the file operation table
    uint32_t inode_idx;
    uint32_t f_pos;
    uint32_t flags;     // 1 if in use, 0 not in use
} file_struct_t;

// the file array struct
typedef struct file_arr_t {
    int32_t n_opend_files;   // the first and second file struct are for stdin and stdout
    file_struct_t files[N_FILE_LIMIT];  // the file struct array
} file_arr_t;

// dir.entry struct
typedef struct dentry {
    uint8_t f_name[F_NAME_LIMIT];
    uint32_t f_type;
    uint32_t inode_idx;
    uint8_t reserved[24];
} dentry_t;

// boot block struct
typedef struct boot_block {
    uint32_t n_dentries;
    uint32_t n_inodes;
    uint32_t n_dblocks;
    uint8_t reserved[52];
    dentry_t dentries[N_DENTRY_LIMIT];
} boot_block_t;

// inode block struct
typedef struct inode_block {
    uint32_t length_in_B;
    uint32_t data_block_idx[INODE_N_DBLOCK_LIMIT];
} inode_block_t;

typedef struct data_block {
    uint8_t content[BLOCK_SIZE_IN_B];
} data_block_t;

/* function headers */
int32_t file_sys_init(module_t *f_sys_mod);

int32_t init_file_arr(file_arr_t *file_arr);
int32_t close_all_files(file_arr_t* file_arr);

int32_t allocate_fd();
int32_t get_file_length(dentry_t *dentry);

int32_t read_dentry_by_name (const uint8_t *fname, dentry_t *dentry);
int32_t read_dentry_by_index(uint32_t index, dentry_t *dentry);
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t *buf, uint32_t bufsize);
int32_t read_dentry_by_inode(uint32_t inode, dentry_t *dentry);

int32_t file_open(const uint8_t *f_name);
int32_t file_close(int32_t fd);
int32_t file_read(int32_t fd, void *buf, int32_t bufsize);
int32_t file_write(int32_t fd, const void *buf, int32_t bufsize);

int32_t dir_write(int32_t fd, const void* buf, int32_t bufsize);
int32_t dir_close(int32_t fd);
int32_t dir_open(const uint8_t *f_name);
int32_t dir_read(int32_t fd, void *buf, int32_t bufsize);

int32_t file_rtc_open(const uint8_t *f_name);
int32_t file_rtc_close(int32_t fd);
int32_t file_rtc_read(int32_t fd, void *buf, int32_t bufsize);
int32_t file_rtc_write(int32_t fd, const void *buf, int32_t bufsize);

int32_t file_audio_open(const uint8_t *f_name);
int32_t file_audio_close(int32_t fd);
int32_t file_audio_read(int32_t fd, void *buf, int32_t bufsize);
int32_t file_audio_write(int32_t fd, const void *buf, int32_t bufsize);
int32_t file_audio_ioctl(int32_t fd, int32_t cmd);

#endif      // FILE_SYS_H
