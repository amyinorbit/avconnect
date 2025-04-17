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

void cmd_mgr_init();
void cmd_mgr_fini();

size_t cmd_mgr_get_output(char *out, size_t cap);
void cmd_mgr_send_cmd_start(int16_t cmd);
void cmd_mgr_send_arg_int(int16_t arg);
void cmd_mgr_send_arg_bool(bool arg);
void cmd_mgr_send_arg_cstr(const char *str);
void cmd_mgr_send_cmd_commit();

void cmd_mgr_proccess_input(const char *buf, size_t len);
bool cmd_mgr_get_cmd(int16_t *out);
bool cmd_mgr_get_arg_int(int16_t *out);
bool cmd_mgr_get_arg_bool(bool *out);
int cmd_mgr_get_arg_cstr(char *buf, size_t len);

#endif /* ifndef _CMD_MGR_H_ */
