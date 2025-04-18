/*===--------------------------------------------------------------------------------------------===
 * str_buf.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "str_buf.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STR_BUF_DEFAULT_CAP     (64)
#define STR_BUF_GROW_FACTOR     (1.4)

void str_buf_init(str_buf_t *buf) {
    buf->data = NULL;
    buf->size = 0;
    buf->cap = 0;
}

void str_buf_fini(str_buf_t *buf) {
    if(buf->cap > 0)
        free(buf->data);
    buf->data = NULL;
    buf->size = 0;
    buf->cap = 0;
}

void str_buf_clear(str_buf_t *buf) {
    buf->size = 0;
    buf->data[0] = '\0';
}

static void str_buf_ensure(str_buf_t *buf, int cap) {
    if(cap <= buf->cap)
        return;
    int new_cap = buf->cap == 0
        ? STR_BUF_DEFAULT_CAP
        : buf->cap;
    while(new_cap < cap)
        new_cap *= STR_BUF_GROW_FACTOR;
    
    buf->data = realloc(buf->data, new_cap);
    assert(buf->data && "failure to allocate data");
    buf->cap = new_cap;
}

void str_buf_push_back(str_buf_t *buf, const char *str, int len) {
    str_buf_ensure(buf, buf->size + len + 1);
    
    memcpy(buf->data + buf->size, str, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
}

void str_buf_printf_back(str_buf_t *buf, const char *fmt, ...) {
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    
    int len = (int)vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    str_buf_ensure(buf, buf->size + len + 1);
    vsnprintf(buf->data + buf->size, len + 1, fmt, args);
    buf->size += len;
    buf->data[buf->size] = '\0';
    
    va_end(args);
}

void str_buf_pop_front(str_buf_t *buf, int num) {
    int to_remove = num > buf->size ? buf->size : num;
    memmove(buf->data, buf->data + to_remove, (buf->size - to_remove));
    buf->size -= to_remove;
    buf->data[buf->size] = '\0';
}

char *str_buf_get(str_buf_t *buf) {
    return buf->data;
}

int str_buf_get_size(const str_buf_t *buf) {
    return buf->size;
}

char *str_buf_take(str_buf_t *buf) {
    char *data = buf->data;
    buf->data = NULL;
    buf->size = 0;
    buf->cap = 0;
    return data;
}

void str_buf_free_data(char *str) {
    free(str);
}
