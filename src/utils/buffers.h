/*===--------------------------------------------------------------------------------------------===
 * buffers.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _BUFFERS_H_
#define _BUFFERS_H_

#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>
#include "../bindings/inputs.h"
#include "../bindings/outputs.h"

#define BUFFER_DEFAULT_CAPACITY     8
#define BUFFER_GROW_FACTOR          1.5

#define DECLARE_BUFFER(name, T)                                                                    \
    typedef struct {                                                                               \
        T       *data;                                                                             \
        int32_t count;                                                                             \
        int32_t capacity;                                                                          \
    } name##_buf_t;                                                                                \
    void name##_buf_init(name##_buf_t* buffer);                                                    \
    void name##_buf_fini(name##_buf_t* buffer);                                                    \
    void name##_buf_fill(name##_buf_t* buffer, T data,  int32_t count);                            \
    void name##_buf_write(name##_buf_t* buffer, T data);                                           \
    void name##_buf_remove(name##_buf_t* buffer, int32_t n);                                       

// This should be used once for each T instantiation, somewhere in a .c file.
#define DEFINE_BUFFER(name, T)                                                                     \
    void name##_buf_init(name##_buf_t* buffer) {                                                   \
        buffer->data = NULL;                                                                       \
        buffer->count = 0;                                                                         \
        buffer->capacity = 0;                                                                      \
    }                                                                                              \
                                                                                                   \
    void name##_buf_fini(name##_buf_t* buffer) {                                                   \
        if(buffer->data)                                                                           \
            free(buffer->data);                                                                    \
        name##_buf_init(buffer);                                                                   \
    }                                                                                              \
                                                                                                   \
    void name##_buf_fill(name##_buf_t* buffer, T data, int32_t count) {                            \
        if(buffer->count + count > buffer->capacity) {                                             \
            while(buffer->count + count > buffer->capacity) {                                      \
                buffer->capacity = buffer->capacity == 0 ?                                         \
                    BUFFER_DEFAULT_CAPACITY : buffer->capacity * BUFFER_GROW_FACTOR;               \
            }                                                                                      \
            buffer->data = safe_realloc(buffer->data, buffer->capacity * sizeof(T));               \
        }                                                                                          \
        for(int i = 0; i < count; i++) {                                                           \
            buffer->data[buffer->count++] = data;                                                  \
        }                                                                                          \
    }                                                                                              \
                                                                                                   \
    void name##_buf_write(name##_buf_t* buffer, T data) {                                          \
        name##_buf_fill(buffer, data, 1);                                                          \
    }                                                                                              \
                                                                                                   \
    void name##_buf_remove(name##_buf_t* buffer, int32_t n) {                                      \
        assert(n >= 0 && n < buffer->count);                                                       \
        memmove(&buffer->data[n], &buffer->data[n + 1], sizeof(T) * (buffer->count - n));          \
        buffer->count -=1;                                                                         \
    }                                                                                              \

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef _BUFFERS_H_ */
