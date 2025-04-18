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
#include "../bindings/inputs.h"
#include "../bindings/outputs.h"


#define DECLARE_BUFFER(name, T, N)                                                                 \
    typedef struct {                                                                               \
        T data[N];                                                                                 \
        int32_t count;                                                                             \
    } name##_buf_t;                                                                                \
    void name##_buf_init(name##_buf_t* buffer);                                                    \
    void name##_buf_fini(name##_buf_t* buffer);                                                    \
    T* name##_buf_add(name##_buf_t* buffer);                                                       \
    void name##_buf_fill(name##_buf_t* buffer, T data,  int32_t count);                            \
    void name##_buf_write(name##_buf_t* buffer, T data);                                           \
    void name##_buf_remove(name##_buf_t* buffer, int32_t n);                                       

// This should be used once for each T instantiation, somewhere in a .c file.
#define DEFINE_BUFFER(name, T, N)                                                                  \
    void name##_buf_init(name##_buf_t* buffer) {                                                   \
        memset(buffer->data, 0, sizeof(buffer->data));                                             \
        buffer->count = 0;                                                                         \
    }                                                                                              \
                                                                                                   \
    void name##_buf_fini(name##_buf_t* buffer) {                                                   \
        name##_buf_init(buffer);                                                                   \
    }                                                                                              \
                                                                                                   \
                                                                                                   \
    T* name##_buf_add(name##_buf_t* buffer) {                                                      \
        assert(buffer->count < N);                                                                 \
        return &buffer->data[buffer->count++];                                                     \
    }                                                                                              \
                                                                                                   \
    void name##_buf_fill(name##_buf_t* buffer, T data, int count) {                                \
        assert(buffer->count + count < N);                                                         \
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
        assert(n < N);                                                                             \
        memmove(buffer->data + n, buffer->data + n + 1, sizeof(T) * (buffer->count - n));          \
        buffer->count -=1;                                                                         \
    }                                                                                              \

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif

#endif /* ifndef _BUFFERS_H_ */
