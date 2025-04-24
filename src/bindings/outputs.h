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
    
#define AV_SREG_MAX_PINS   (16)
    
typedef enum {
    AV_TYPE_INVALID,
    AV_TYPE_INT,
    AV_TYPE_FLOAT,
    AV_TYPE_DOUBLE,
} av_dr_type_t;

typedef enum {
    AV_OP_NEQ,
    AV_OP_EQ,
    AV_OP_LT,
    AV_OP_LTEQ,
    AV_OP_GT,
    AV_OP_GTEQ,
    AV_OP_TEST,
} av_cmp_op_t;

static const char *av_cmp_str[] = {
    [AV_OP_NEQ]     = "!=",
    [AV_OP_EQ]      = "==",
    [AV_OP_LT]      = "<",
    [AV_OP_LTEQ]    = "<=",
    [AV_OP_GT]      = ">",
    [AV_OP_GTEQ]    = ">=",
    [AV_OP_TEST]    = "&",
};

typedef enum {
    AV_OP_MULT,
    AV_OP_PLUS,
    AV_OP_MINUS,
} av_mod_op_t;

static const char *av_mod_str[] = {
    [AV_OP_MULT]    = "*",
    [AV_OP_PLUS]    = "+",
    [AV_OP_MINUS]   = "-",
};

typedef struct {
    char            path[128];
    bool            has_changed;
    bool            has_resolved;
    XPLMDataRef     ref;
    av_dr_type_t    type;
} av_dref_t;

typedef enum {
    AV_OUT_PWM,
    AV_OUT_SHIFT_REG,
} av_out_type_t;

typedef struct {
    av_out_type_t   type;
    int             id;
} av_out_t;

typedef struct {
    av_dref_t       dref;
    av_cmp_op_t     cmp_op;
    float           cmp_val;
    int             last_out;
} av_out_sreg_pin_t;

typedef struct {
    av_out_t            base;
    av_out_sreg_pin_t   pins[AV_SREG_MAX_PINS];
} av_out_sreg_t;

typedef struct {
    av_out_t            base;
    av_dref_t           dref;
    av_mod_op_t         mod_op;
    float               mod_val;
    int                 last_out;
} av_out_pwm_t;


static inline void av_dref_init(av_dref_t *dref) {
    dref->path[0] = '\0';
    dref->has_resolved = false;
    dref->has_changed = true;
    dref->ref = NULL;
    dref->type = AV_TYPE_INVALID;
}

#ifdef __cplusplus
}
#endif


#endif /* ifndef _OUTPUTS_H_ */


