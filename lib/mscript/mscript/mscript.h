/*===--------------------------------------------------------------------------------------------===
 * mscript.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _MSCRIPT_H_
#define _MSCRIPT_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#ifndef MSCRIPT_MAX_CONSTANTS
#define MSCRIPT_MAX_CONSTANTS       (16)
#endif
    
#ifndef MSCRIPT_MAX_BYTECODE_SIZE
#define MSCRIPT_MAX_BYTECODE_SIZE   (64)
#endif
    
#ifndef MSCRIPT_MAX_STACK_SIZE
#define MSCRIPT_MAX_STACK_SIZE      (64)
#endif

typedef enum {
    MSCRIPT_TYPE_INT,
    MSCRIPT_TYPE_BOOL,
    MSCRIPT_TYPE_FLOAT,
    MSCRIPT_TYPE_NIL,
} mscript_type_t;

typedef struct {
    mscript_type_t  type;
    union {
        bool        bval;
        int32_t     ival;
        float       fval;
    };
} mscript_val_t;

typedef struct {
    void        (*log_fn)(const char *msg, void *ptr);
    void        *user_data;
} mscript_conf_t;

#define MSCRIPT_DEF_CONF ((mscript_conf_t){NULL, NULL})

typedef struct mscript_t mscript_t;

mscript_t *mscript_new(const mscript_conf_t *conf);
void mscript_destroy(mscript_t *vm);

bool mscript_compile(mscript_t *vm, const char *src, int len);
mscript_val_t mscript_run(mscript_t *vm, const mscript_val_t inputs[num_inputs], int num_inputs);


#ifdef __cplusplus
}
#endif

#endif /* ifndef _MSCRIPT_H_ */
