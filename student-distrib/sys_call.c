#include "file_sys.h"
#include "process.h"
#include "lib.h"
#include "page_lib.h"
#include "tests.h"
#include "signal_sys_call.h"
#include "scheduler.h"
#include "sound_card.h"

#define FILE_CLOSED_TEST 1

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
    pcb_t *cur_pcb;

    cur_pcb = get_cur_process();

    if (cur_pcb == NULL) {
        printf("ERROR [SYS FILE] in sys_open: the current process is NULL");
        return -1;
    }
    if (cur_pcb->file_arr.n_opend_files >= N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_open: cannot OPEN file [%s] because the max number of files is reached\n", f_name);
        return -1;
    }

    if (f_name == NULL) {
        printf("ERROR [SYS FILE] in sys_open: f_name NULL pointer\n");
        return -1;
    } else if (strncmp((int8_t*) "audio", (int8_t*) f_name, (uint32_t) 6) == 0) {
        fd = file_audio_open(f_name);
    } else {
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
    }

    if (fd != -1) cur_pcb->file_arr.n_opend_files++;

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
    pcb_t *cur_pcb;

    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_close: invalid fd\n");
        return -1;
    }
    if (fd == 0) {printf("ERROR [FILE]: cannot CLOSE stdin\n"); return -1;}
    if (fd == 1) {printf("ERROR [FILE]: cannot CLOSE stdout\n"); return -1;}

    // get the current process
    cur_pcb = get_cur_process();

    if (cur_pcb == NULL) {
        printf("ERROR [SYS FILE] in sys_close: the current process is NULL");
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].f_op == NULL) {
        printf("ERROR [SYS FILE] in sys_close: the file has no file operation. fd: %d\n", fd);
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [FILE]: cannot CLOSE a file that is not opened. fd: %d\n", fd);
        return -1;
    }

    ret = cur_pcb->file_arr.files[fd].f_op->close(fd);

    cur_pcb->file_arr.files[fd].flags = AVAILABLE;  // empty the block
    cur_pcb->file_arr.n_opend_files--;

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
    pcb_t *cur_pcb = get_cur_process();

    // bad input checking
    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_read: invalid fd\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR [SYS FILE] in sys_read: read buffer NULL pointer\n");
        return -1;
    }
    if (bufsize < 0) {
        printf("ERROR [SYS FILE] in sys_read: read buffer size should not be negative\n");
        return -1;
    }
    if (cur_pcb == NULL) {
        printf("ERROR [SYS FILE] in sys_read: the current process is NULL");
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_read: cannot READ a file that is not opened. fd: %d\n", fd);
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].f_op == NULL) {
        printf("ERROR [SYS FILE] in sys_read: the file has no file operation. fd: %d\n", fd);
        return -1;
    }
    // stdout is write-only
    if (fd == 1) {
        printf("ERROR [SYS FILE] in sys_read: stdout is write-only\n");
        return -1;
    }

    /* for debug */
    // dentry_t dentry;
    // read_dentry_by_inode(file_arr[fd].inode_idx, &dentry);
    // printf("In file_test: Now the file type is %d\n", dentry.f_type);

    return cur_pcb->file_arr.files[fd].f_op->read(fd, buf, bufsize);
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
    pcb_t *cur_pcb = get_cur_process();

    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_write: invalid fd\n");
        return -1;
    }
    if (buf == NULL) {
        printf("ERROR [SYS FILE] in sys_write: write buffer NULL pointer\n");
        return -1;
    }
    if (bufsize < 0) {
        printf("ERROR [SYS FILE] in sys_write: write buffer size should be non-negative\n");
        return -1;
    }
    if (cur_pcb == NULL) {
        printf("ERROR [SYS FILE] in sys_write: the current process is NULL");
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].f_op == NULL) {
        printf("ERROR [SYS FILE] in sys_write: the file has no file operation. fd: %d\n", fd);
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_write: the file is not opened. fd: %d\n", fd);
        return -1;
    }
    // stdin is read-only
    if (fd == 0) {
        printf("ERROR [SYS FILE] in sys_write: stdin is read-only\n");
        return -1;
    }
    return cur_pcb->file_arr.files[fd].f_op->write(fd, buf, bufsize);
}

/**
 * sys_ioctl
 * Description: system call: system io control for the device
 * Input: fd - the file descriptor
          cmd - the command sent to the device
 * Output: -1: fail
 * Side effect: a command is sent to the device
 */
int32_t sys_ioctl(int32_t fd, int32_t cmd) {
    pcb_t *cur_pcb = get_cur_process();

    if (fd < 0 || fd > N_FILE_LIMIT) {
        printf("ERROR [SYS FILE] in sys_ioctl: invalid fd\n");
        return -1;
    }
    if (cur_pcb == NULL) {
        printf("ERROR [SYS FILE] in sys_ioctl: the current process is NULL");
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].flags == AVAILABLE) {
        printf("WARNING [SYS FILE] in sys_ioctl: the file is not opened. fd: %d\n", fd);
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].f_op == NULL) {
        printf("ERROR [SYS FILE] in sys_ioctl: the file has no file operation. fd: %d\n", fd);
        return -1;
    }
    if (cur_pcb->file_arr.files[fd].f_op->ioctl == NULL) {
        printf("ERROR [SYS FILE] in sys_ioctl: the file has no ioctl operation. fd: %d\n", fd);
        return -1;
    }
    return cur_pcb->file_arr.files[fd].f_op->ioctl(fd, cmd);
}

/**
 * invalid_sys_call
 * Description: for invalid system call number
 * Input: None
 * Output: None
 * Side effect: None
 */
int invalid_sys_call(){
     printf("The system call is invalid!!!! Please check the call number!!! \n");
     return -1;
}

/**
 * sys_vidmap
 * Description: send pointer to vm to user
 * Input: screen_start -- a virtual address in user region, contain a pointer to vm
 * Output: 0 - if success
 *        -1 - if fail
 * Side effect: modify the page directory
 */

int sys_vidmap(uint8_t** screen_start){

    pcb_t *cur_pcb = get_cur_process();
    int check_address = (int) screen_start;

    // check whether screen_start is in user region (virtual 128MB to 132 MB)
    if( (check_address < USER_VA_START) || (check_address >= USER_VA_END) ){
        printf("The screen start virtual address when you call vidmap is invalid!!\n");
        return -1;
    }

    if (cur_pcb == NULL) {
        printf("ERROR in get_cur_process(): the cur_pcb is NULL\n");
        return -1;
    }

    // check if the current process has allocated a terminal
    if (cur_pcb->terminal==NULL || cur_pcb->terminal->id == -1) {
        printf("ERROR in get_cur_process(): the current process has allocated no terminal\n");
        return -1;
    }

    // setup the video memory in PD
//    set_video_memory(cur_pcb->terminal);
    set_video_memory_SVGA(cur_pcb->terminal);   // for SVGA

    *screen_start = (uint8_t*)(USER_VA_END + VM_INDEX);

    return 0;
}

/**
 * sys_get_args
 * Description: get args to user
 * Input: buf -- user buffer
 *        nbytes -- num of byte
 * Output: 0 - if success
 *        -1 - if fail
 * Side effect: None
 */
int32_t sys_get_args(uint8_t *buf, int32_t nbytes){

    // sanity check
    if(buf == NULL || nbytes < 0) return -1;

    uint8_t* cur_arg = get_cur_process()->args;

    // check arg is correct
    if(cur_arg == NULL || (strlen((int8_t *) cur_arg) > nbytes)) return -1;

    // copy the arg, change to int8_t to meet the requirement of strnpy
    strncpy((int8_t *) buf, (int8_t *) cur_arg, nbytes);

    return 0;
}

/**
 * sys_play_sound
 * Description: Play sound using built in speaker
 * Input: nFrequence -- number of frequence
 * Output: always 0
 * Side effect: make sound
 */
/* reference: https://wiki.osdev.org/PC_Speaker */
/* NOTE: all the magic numbers are provided on the net and without any explanation */
int32_t sys_play_sound(uint32_t nFrequence) {
    uint32_t divide;
    uint8_t temp;

    if (nFrequence == 0) return 0;

    // Set the PIT to the desired frequency
    divide = 1193180 / nFrequence; 
    outb(0xb6, 0x43);
    outb((uint8_t) (divide), 0x42);
    outb((uint8_t) (divide >> 8), 0x42);

    // play the sound using the PC speaker
    temp = inb(0x61);
    if (temp != (temp | 3)) {
        outb(temp | 3, 0x61);
    }

    return 0;
}

/**
 * sys_nosound
 * Description: stop the sound
 * Input: None
 * Output: always 0
 * Side effect: stop sound
 */
/* reference: https://wiki.osdev.org/PC_Speaker */
/* NOTE: all the magic numbers are provided on the net and without any explanation */
int32_t sys_nosound() {
    uint8_t temp = inb(0x61) & 0xFC;
    outb(temp, 0x61);
    return 0;
}
