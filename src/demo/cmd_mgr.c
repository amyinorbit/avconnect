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
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static struct {
    bool            sending;
    str_buf_t       buf_in;
    str_buf_t       buf_out;
    
    const char      *cmd_end;
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
    str_buf_printf_back(&cmd_mgr.buf_out, "%d", (int)cmd);
    cmd_mgr.sending = true;
}

void cmd_mgr_send_arg_int(int16_t arg) {
    if(!cmd_mgr.sending)
        return;
    str_buf_printf_back(&cmd_mgr.buf_out, ",%d", (int)arg);
}

void cmd_mgr_send_arg_bool(bool arg) {
    if(!cmd_mgr.sending)
        return;
    str_buf_printf_back(&cmd_mgr.buf_out, ",%s", arg ? "true" : "false");
}

void cmd_mgr_send_arg_cstr(const char *str) {
    if(!cmd_mgr.sending)
        return;
    // TODO: we should probably be escaping data here.
    str_buf_printf_back(&cmd_mgr.buf_out, ",%s", str);
}

void cmd_mgr_send_cmd_commit() {
    if(!cmd_mgr.sending)
        return;
    str_buf_push_back(&cmd_mgr.buf_out, ";", 1);
    cmd_mgr.sending = false;
}

void cmd_mgr_proccess_input(const char *str, size_t len) {
    str_buf_push_back(&cmd_mgr.buf_in, str, len);
}

static char *skip_white_space(char *str) {
    while(isspace(*str))
        str += 1;
    return *str != '\0' ? str : NULL;
}

static char *get_param_end(char *str) {
    while(true) {
        if(*str == '\0' || *str == ';' || *str == ',')
            return str;
        str += 1;
    }
    return NULL;
}

static char *get_cmd_end(char *str) {
    while(true) {
        if(*str == '\0')
            return NULL;
        if(*str == ';')
            break;
        str += 1;
    }
    return str;
}

typedef struct {
    char *start;
    char *end;
    bool is_last;
} token_t;

static bool get_next_token(char *str, const char *cmd_end, token_t *tok) {
    if(cmd_end == NULL)
        return false;
    char *start = skip_white_space(str);
    if(start == NULL)
        return false;
    char *end = get_param_end(start);
    
    if(end == NULL || end == start)
        return false;
    
    tok->start = start;
    tok->end = end;
    tok->is_last = end == cmd_end;
    return true;
}

int16_t cmd_mgr_get_cmd() {
    char *str = str_buf_get(&cmd_mgr.buf_in);
    cmd_mgr.cmd_end = get_cmd_end(str);
    if(cmd_mgr.cmd_end == NULL)
        return -1;
    return cmd_mgr_get_arg_int();
}

int16_t cmd_mgr_get_arg_int() {
    token_t tok;
    char *str = str_buf_get(&cmd_mgr.buf_in);
    const char *end = cmd_mgr.cmd_end;
    if(!get_next_token(str, end, &tok))
        return 0;
    
    *tok.end = '\0';
    int16_t val = (int16_t)atoi(tok.start);
    
    str_buf_pop_front(&cmd_mgr.buf_in, (tok.end - str) + 1);
    return val;
}

bool cmd_mgr_get_arg_bool() {
    token_t tok;
    char *str = str_buf_get(&cmd_mgr.buf_in);
    const char *end = cmd_mgr.cmd_end;
    if(!get_next_token(str, end, &tok))
        return false;
    
    *tok.end = '\0';
    bool val = strcmp(tok.start, "true") == 0;
    
    str_buf_pop_front(&cmd_mgr.buf_in, (tok.end - str) + 1);
    return val;
}

size_t cmd_mgr_get_arg_str(char *buf, size_t cap) {
    token_t tok;
    char *str = str_buf_get(&cmd_mgr.buf_in);
    const char *end = cmd_mgr.cmd_end;
    if(!get_next_token(str, end, &tok))
        return false;
    
    *tok.end = '\0';
    
    size_t len = tok.end - tok.start;
    size_t to_copy = len > cap-1 ? cap - 1 : len;
    memcpy(buf, tok.start, to_copy);
    buf[to_copy] = '\0';
    
    str_buf_pop_front(&cmd_mgr.buf_in, len + 1);
    return true;
}
