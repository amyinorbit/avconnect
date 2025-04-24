/*===--------------------------------------------------------------------------------------------===
 * outputs.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _OUTPUTS_H_
#define _OUTPUTS_H_

#include <XPLMDataAccess.h>

#ifdef __cplusplus
extern "C" {
#endif
    
#define AV_SHIFT_REG_MAX_PINS   (16)
    
typedef enum {
    AV_TYPE_INT,
    AV_TYPE_FLOAT,
    AV_TYPE_INVALID,
} av_dr_type_t;
    
typedef enum {
    AV_EQ_NEQ,
    AV_EQ,
    AV_OP_LT,
    AV_OP_LTEQ,
    AV_OP_GT,
    AV_OP_GTEQ,
    AV_OP_TEST,
} av_cmp_op_t;

typedef struct {
    char            path[128];
    bool            has_changed;
    bool            has_resolved;
    XPLMDataRef     ref;
    av_dr_type_t    type;
    av_cmp_op_t     cmp_op;
    float           cmp_val;
    int             last_output;
} av_dr_t;

typedef enum {
    AV_OUT_PWM,
    AV_OUT_SHIFT_REG,
} av_out_type_t;

typedef struct {
    av_out_type_t   type;
    char            name[32];
    int             id;
} av_out_t;

typedef struct {
    av_out_t        base;
    av_dr_t         dr[AV_SHIFT_REG_MAX_PINS];
} av_out_sreg_t;


static inline void av_dr_init(av_dr_t *dr) {
    memset(dr, 0, sizeof(*dr));
    dr->has_changed = true;
}

#ifdef __cplusplus
}
#endif


#endif /* ifndef _OUTPUTS_H_ */


