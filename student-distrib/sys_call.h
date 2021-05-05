#ifndef     _SYS_CALL_H
#define     _SYS_CALL_H

#include "types.h"

int32_t sys_open(const uint8_t *f_name);
int32_t sys_close(int32_t fd);
int32_t sys_read(int32_t fd, void *buf, int32_t bufsize);
int32_t sys_write(int32_t fd, const void *buf, int32_t bufsize);
int32_t sys_ioctl(int32_t fd, int32_t cmd);
int invalid_sys_call();

int32_t sys_get_args(uint8_t *buf, int32_t nbytes);

int sys_vidmap(uint8_t** screen_start);

int32_t sys_play_sound(uint32_t nFrequence);
int32_t sys_nosound();

void load_esp_and_return(uint32_t new_esp,int32_t status);

#endif
