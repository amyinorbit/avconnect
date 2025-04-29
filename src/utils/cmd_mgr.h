/*===--------------------------------------------------------------------------------------------===
 * cmd_mgr.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _CMD_MGR_H_
#define _CMD_MGR_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "str_buf.h"

#define CMD_MGR_DEBUG 0

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct {
    bool            sending;
    str_buf_t       buf_in;
    str_buf_t       buf_out;
    const char      *cmd_end;
} cmd_mgr_t;

void cmd_mgr_init(cmd_mgr_t *mgr);
void cmd_mgr_fini(cmd_mgr_t *mgr);

int cmd_mgr_get_output(cmd_mgr_t *mgr, char *out, int cap);
void cmd_mgr_send_cmd_start(cmd_mgr_t *mgr, int16_t cmd);
void cmd_mgr_send_arg_int(cmd_mgr_t *mgr, int16_t arg);
void cmd_mgr_send_arg_bool(cmd_mgr_t *mgr, bool arg);
void cmd_mgr_send_arg_cstr(cmd_mgr_t *mgr, const char *str);
void cmd_mgr_send_cmd_commit(cmd_mgr_t *mgr);

void cmd_mgr_proccess_input(cmd_mgr_t *mgr, const char *buf, int len);
int16_t cmd_mgr_get_cmd(cmd_mgr_t *mgr);
int16_t cmd_mgr_get_arg_int(cmd_mgr_t *mgr);
bool cmd_mgr_get_arg_bool(cmd_mgr_t *mgr);
int cmd_mgr_get_arg_str(cmd_mgr_t *mgr, char *buf, int len);
void cmd_mgr_skip_cmd(cmd_mgr_t *mgr);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _CMD_MGR_H_ */
