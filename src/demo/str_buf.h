/*===--------------------------------------------------------------------------------------------===
 * str_buf.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _STR_BUF_H_
#define _STR_BUF_H_

#include <stddef.h>

// TODO: defined alternate, static-memory-based version for Raspberry Pi Pico
typedef struct {
    char    *data;
    size_t  cap;
    size_t  size;
} str_buf_t;

void str_buf_init(str_buf_t *buf);
void str_buf_fini(str_buf_t *buf);

void str_buf_clear(str_buf_t *buf);

void str_buf_push_back(str_buf_t *buf, const char *str, size_t len);
void str_buf_printf_back(str_buf_t *buf, const char *fmt, ...);

void str_buf_pop_front(str_buf_t *buf, size_t num);

size_t str_buf_get_size(const str_buf_t *buf);
char *str_buf_get(str_buf_t *buf);
char *str_buf_take(str_buf_t *buf);
void str_buf_free_data(char *buf);

#endif /* ifndef _STR_BUF_H_ */
