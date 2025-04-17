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
#include <stdio.h>
#include <stdarg.h>
#include <acfutils/safe_alloc.h>

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

static void str_buf_ensure(str_buf_t *buf, size_t cap) {
    if(cap <= buf->cap)
        return;
    size_t new_cap = buf->cap == 0 ? STR_BUF_DEFAULT_CAP : buf->cap;
    while(new_cap < cap)
        new_cap *= STR_BUF_GROW_FACTOR;
    
    buf->data = safe_realloc(buf->data, new_cap);
    buf->cap = new_cap;
}

void str_buf_add(str_buf_t *buf, const char *str, size_t len) {
    str_buf_ensure(buf, buf->size + len + 1);
    
    memcpy(buf->data + buf->size, str, len);
    buf->size += len;
    buf->data[buf->size] = '\0';
}

void str_buf_printf(str_buf_t *buf, const char *fmt, ...) {
    
    
    va_list args, args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);
    
    size_t len = vsnprintf(NULL, 0, fmt, args_copy);
    va_end(args_copy);
    
    str_buf_ensure(buf, buf->size + len + 1);
    vsnprintf(buf->data + buf->size, len + 1, fmt, args);
    buf->size += len;
    buf->data[buf->size] = '\0';
    
    va_end(args);
}

const char *str_buf_get(const str_buf_t *buf) {
    return buf->data;
}

size_t str_buf_get_size(const str_buf_t *buf) {
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
