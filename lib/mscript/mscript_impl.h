/*===--------------------------------------------------------------------------------------------===
 * mscript_impl.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _MSCRIPT_IMPL_H_
#define _MSCRIPT_IMPL_H_

#include <mscript/mscript.h>
#include "tok.h"

#define OPCODE(code, _, __) OP_##code
typedef enum {
#include "op.x.h"
} mscript_op_t;
#undef OPCODE

struct mscript_t {
    uint8_t         *inst_ptr;
    
    
    uint8_t         bytecode[MSCRIPT_MAX_BYTECODE_SIZE];
    int             bytecode_size;
    
    mscript_val_t   constants[MSCRIPT_MAX_CONSTANTS];
    
    mscript_val_t   stack[MSCRIPT_MAX_STACK_SIZE];
    mscript_val_t   *stack_ptr;
    mscript_val_t   *stack_base;
};
 

#endif /* ifndef _MSCRIPT_IMPL_H_ */


