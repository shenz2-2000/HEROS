#ifndef     _SYS_CALL_H
#define     _SYS_CALL_H

#include "types.h"

int32_t sys_open(const uint8_t *f_name);
int32_t sys_close(int32_t fd);
int32_t sys_read(int32_t fd, void *buf, int32_t bufsize);
int32_t sys_write(int32_t fd, const void *buf, int32_t bufsize);
void invalid_sys_call();

int parse_args(uint8_t *command, uint8_t **args);
int32_t system_halt(int32_t status);
int32_t sys_execute(uint8_t *command);

#endif
