/*===--------------------------------------------------------------------------------------------===
 * cmd_mgr.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "cmd_mgr.h"
#include "str_buf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static struct {
    bool        sending;
    str_buf_t   buf_in;
    str_buf_t   buf_out;
} cmd_mgr;

void cmd_mgr_init() {
    str_buf_init(&cmd_mgr.buf_in);
    str_buf_init(&cmd_mgr.buf_out);
}
void cmd_mgr_fini() {
    str_buf_fini(&cmd_mgr.buf_in);
    str_buf_fini(&cmd_mgr.buf_out);
}

size_t cmd_mgr_get_output(char *out, size_t cap) {
    size_t available = str_buf_get_size(&cmd_mgr.buf_out);
    
    if(cap == 0 && out == NULL)
        return available;
    
    const char *data = str_buf_get(&cmd_mgr.buf_out);
    size_t to_copy = available > cap - 1 ? cap - 1 : available;
    
    memcpy(out, data, to_copy);
    out[to_copy] = '\0';
    str_buf_clear(&cmd_mgr.buf_out);
    return to_copy;
}

void cmd_mgr_send_cmd_start(int16_t cmd) {
    if(cmd_mgr.sending)
        cmd_mgr_send_cmd_commit();
    str_buf_printf(&cmd_mgr.buf_out, "%d", (int)cmd);
    cmd_mgr.sending = true;
}

void cmd_mgr_send_arg_int(int16_t arg) {
    if(!cmd_mgr.sending)
        return;
    str_buf_printf(&cmd_mgr.buf_out, ",%d", (int)arg);
}

void cmd_mgr_send_arg_bool(bool arg) {
    if(!cmd_mgr.sending)
        return;
    str_buf_printf(&cmd_mgr.buf_out, ",%s", arg ? "true" : "false");
}

void cmd_mgr_send_arg_cstr(const char *str) {
    if(!cmd_mgr.sending)
        return;
    // TODO: we should probably be escaping data here.
    str_buf_printf(&cmd_mgr.buf_out, ",%s", str);
}

void cmd_mgr_send_cmd_commit() {
    if(!cmd_mgr.sending)
        return;
    str_buf_add(&cmd_mgr.buf_out, ";", 1);
    cmd_mgr.sending = false;
}

void cmd_mgr_proccess_input(const char *str, size_t len) {
    str_buf_add(&cmd_mgr.buf_in, str, len);
}

bool cmd_mgr_get_cmd(int16_t *out) {
    
}

bool cmd_mgr_get_arg_int(int16_t *out) {
    
}

bool cmd_mgr_get_arg_bool(bool *out) {
    
}

int cmd_mgr_get_arg_cstr(char *buf, size_t len) {
    
}
