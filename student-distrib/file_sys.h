#ifndef     _FILE_SYS_H
#define     _FILE_SYS_H

#include "types.h"

// consts
#define F_NAME_LIMIT    8

#define BLOCK_SIZE_IN_B             4096
#define STATS_ENTRY_SIZE_IN_B       64
#define DENTRY_SIZE_IN_B            64

#define N_DENTRY_LIMIT_IN_B         (BLOCK_SIZE_IN_B - STATS_ENTRY_SIZE_IN_B) / DENTRY_SIZE_IN_B
#define INODE_N_DBLOCK_LIMIT_IN_B   (BLOCK_SIZE_IN_B - 4) / 4   // 4: an entry in inode block takes 4 bytes

// file operation table
typedef struct file_operations {
    int32_t (*open) (const uint8_t *f_name);
    int32_t (*close) (int32_t fd);
    int32_t (*read) (int32_t fd, void *buf, int32_t bufsize);
    int32_t (*write) (int32_t fd, const void *buf, int32_t bufsize);
} file_operations_t;

// process control block (PCB), entries in the file array
typedef struct pcb {
    file_operations_t *f_op;
    uint32_t inode_idx;
    uint32_t f_pos;
    uint32_t flags;
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
    uint32_t n_dentry;
    uint32_t n_inodes;
    uint32_t n_dblock;
    uint8_t reserved[52];
    dentry_t dentries[N_DENTRY_LIMIT_IN_B];
} boot_block_t;

// inode block struct
typedef struct inode_block {
    uint32_t length_in_B;
    uint32_t data_block_idx[INODE_N_DBLOCK_LIMIT_IN_B];
} inode_block;

typedef struct data_block {
    uint8_t content[BLOCK_SIZE_IN_B];
} data_block_t;

#endif      // FILE_SYS_H
