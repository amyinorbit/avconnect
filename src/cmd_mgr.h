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

#ifdef __cplusplus
extern "C" {
#endif

void cmd_mgr_init();
void cmd_mgr_fini();

int cmd_mgr_get_output(char *out, int cap);
void cmd_mgr_send_cmd_start(int16_t cmd);
void cmd_mgr_send_arg_int(int16_t arg);
void cmd_mgr_send_arg_bool(bool arg);
void cmd_mgr_send_arg_cstr(const char *str);
void cmd_mgr_send_cmd_commit();

void cmd_mgr_proccess_input(const char *buf, int len);
int16_t cmd_mgr_get_cmd();
int16_t cmd_mgr_get_arg_int();
bool cmd_mgr_get_arg_bool();
int cmd_mgr_get_arg_str(char *buf, int len);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _CMD_MGR_H_ */
