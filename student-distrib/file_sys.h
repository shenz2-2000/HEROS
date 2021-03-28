#ifndef     _FILE_SYS_H
#define     _FILE_SYS_H

#include "types.h"
#include "multiboot.h"

// consts
#define F_NAME_LIMIT    32

#define BLOCK_SIZE_IN_B             4096
#define STATS_ENTRY_SIZE_IN_B       64
#define DENTRY_SIZE_IN_B            64

#define N_DENTRY_LIMIT              (BLOCK_SIZE_IN_B - STATS_ENTRY_SIZE_IN_B) / DENTRY_SIZE_IN_B
#define INODE_N_DBLOCK_LIMIT        (BLOCK_SIZE_IN_B - 4) / 4   // 4: an entry in inode block takes 4 bytes

#define N_PCB_LIMIT     8

#define OCCUPIED  1
#define AVAILABLE 0

// file operation table
typedef struct file_operations {
    int32_t (*open) (const uint8_t *f_name);
    int32_t (*close) (int32_t fd);
    int32_t (*read) (int32_t fd, void *buf, int32_t bufsize);
    int32_t (*write) (int32_t fd, const void *buf, int32_t bufsize);
} file_operations_t;

// process control block (PCB), entries in the file array
typedef struct pcb {
    file_operations_t *f_op;    // the file operation table
    uint32_t inode_idx;
    uint32_t f_pos;
    uint32_t flags;     // 1 if in use, 0 not in use
} pcb_t;

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
int32_t allocate_fd();

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

int32_t sys_open(const uint8_t *f_name);
int32_t sys_close(int32_t fd);
int32_t sys_read(int32_t fd, void *buf, int32_t bufsize);
int32_t sys_write(int32_t fd, const void *buf, int32_t bufsize);

#endif      // FILE_SYS_H
