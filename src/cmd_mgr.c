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

void cmd_mgr_init(cmd_mgr_t *mgr) {
    str_buf_init(&mgr->buf_in);
    str_buf_init(&mgr->buf_out);
    mgr->cmd_end = NULL;
    mgr->sending = false;
}
void cmd_mgr_fini(cmd_mgr_t *mgr) {
    str_buf_fini(&mgr->buf_in);
    str_buf_fini(&mgr->buf_out);
    mgr->cmd_end = NULL;
    mgr->sending = false;
}

int cmd_mgr_get_output(cmd_mgr_t *mgr, char *out, int cap) {
    int available = str_buf_get_size(&mgr->buf_out);
    
    if(cap == 0 && out == NULL)
        return available;
    
    const char *data = str_buf_get(&mgr->buf_out);
    int to_copy = available > cap - 1 ? cap - 1 : available;
    
    memcpy(out, data, to_copy);
    out[to_copy] = '\0';
    str_buf_clear(&mgr->buf_out);
    return to_copy;
}

void cmd_mgr_send_cmd_start(cmd_mgr_t *mgr, int16_t cmd) {
    if(mgr->sending)
        cmd_mgr_send_cmd_commit(mgr);
    str_buf_printf_back(&mgr->buf_out, "%d", (int)cmd);
    mgr->sending = true;
}

void cmd_mgr_send_arg_int(cmd_mgr_t *mgr, int16_t arg) {
    if(!mgr->sending)
        return;
    str_buf_printf_back(&mgr->buf_out, ",%d", (int)arg);
}

void cmd_mgr_send_arg_bool(cmd_mgr_t *mgr, bool arg) {
    if(!mgr->sending)
        return;
    str_buf_printf_back(&mgr->buf_out, ",%s", arg ? "true" : "false");
}

void cmd_mgr_send_arg_cstr(cmd_mgr_t *mgr, const char *str) {
    if(!mgr->sending)
        return;
    // TODO: we should probably be escaping data here.
    str_buf_printf_back(&mgr->buf_out, ",%s", str);
}

void cmd_mgr_send_cmd_commit(cmd_mgr_t *mgr) {
    if(!mgr->sending)
        return;
    str_buf_push_back(&mgr->buf_out, ";\r\n", 3);
    mgr->sending = false;
}

void cmd_mgr_proccess_input(cmd_mgr_t *mgr, const char *str, int len) {
    str_buf_push_back(&mgr->buf_in, str, len);
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

int16_t cmd_mgr_get_cmd(cmd_mgr_t *mgr) {
    char *str = str_buf_get(&mgr->buf_in);
    mgr->cmd_end = get_cmd_end(str);
    if(mgr->cmd_end == NULL)
        return -1;
    return cmd_mgr_get_arg_int(mgr);
}

int16_t cmd_mgr_get_arg_int(cmd_mgr_t *mgr) {
    token_t tok;
    char *str = str_buf_get(&mgr->buf_in);
    const char *end = mgr->cmd_end;
    if(!get_next_token(str, end, &tok))
        return 0;
    
    *tok.end = '\0';
    int16_t val = (int16_t)atoi(tok.start);
    
    str_buf_pop_front(&mgr->buf_in, (tok.end - str) + 1);
    return val;
}

bool cmd_mgr_get_arg_bool(cmd_mgr_t *mgr) {
    token_t tok;
    char *str = str_buf_get(&mgr->buf_in);
    const char *end = mgr->cmd_end;
    if(!get_next_token(str, end, &tok))
        return false;
    
    *tok.end = '\0';
    bool val = strcmp(tok.start, "true") == 0;
    
    str_buf_pop_front(&mgr->buf_in, (tok.end - str) + 1);
    return val;
}

int cmd_mgr_get_arg_str(cmd_mgr_t *mgr, char *buf, int cap) {
    token_t tok;
    char *str = str_buf_get(&mgr->buf_in);
    const char *end = mgr->cmd_end;
    if(!get_next_token(str, end, &tok))
        return false;
    
    *tok.end = '\0';
    
    int len = tok.end - tok.start;
    int to_copy = len > cap-1 ? cap - 1 : len;
    if(cap) {
        memcpy(buf, tok.start, to_copy);
        buf[to_copy] = '\0';
    }
    
    str_buf_pop_front(&mgr->buf_in, len + 1);
    return true;
}
