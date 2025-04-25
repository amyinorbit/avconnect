/*===--------------------------------------------------------------------------------------------===
 * device_output.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "device_impl.h"
#include "cmd_ids.h"
#include <acfutils/assert.h>

static void commit_cmd(cmd_mgr_t *mgr, serial_t *serial);

// MARK: - Output Management

static void reset_pwm(av_device_t *dev, av_out_pwm_t *pwm) {
    if(dev->serial == NULL)
        return;
    pwm->last_out = 0;
    cmd_mgr_send_cmd_start(&dev->mgr, kSetPin);
    cmd_mgr_send_arg_int(&dev->mgr, pwm->base.id);
    cmd_mgr_send_arg_int(&dev->mgr, 0);
    commit_cmd(&dev->mgr, dev->serial);
}

static void reset_sreg(av_device_t *dev, av_out_sreg_t *sreg) {
    if(dev->serial == NULL)
        return;
    char cmd[AV_SREG_MAX_PINS * 4] = "";
    int cmd_len = 0;
    
    for(int i = 0; i < AV_SREG_MAX_PINS; ++i) {
        if(cmd_len > 0)
            cmd[cmd_len++] = '|';
        cmd_len += snprintf(cmd + cmd_len, sizeof(cmd) - cmd_len, "%d", i);
        sreg->pins[i].last_out = 0;
    }
    cmd[cmd_len] = '\0';
    
    cmd_mgr_send_cmd_start(&dev->mgr, kSetShiftRegisterPins);
    cmd_mgr_send_arg_int(&dev->mgr, sreg->base.id);
    cmd_mgr_send_arg_cstr(&dev->mgr, cmd);
    cmd_mgr_send_arg_int(&dev->mgr, 0);
    commit_cmd(&dev->mgr, dev->serial);
}


void av_device_out_reset(av_device_t *dev) {
    for(int i = 0; i < dev->pwms.count; ++i)
        reset_pwm(dev, dev->pwms.data[i]);
    for(int i = 0; i < dev->sregs.count; ++i)
        reset_sreg(dev, dev->sregs.data[i]);
}

int av_device_get_out_count(const av_device_t *dev) {
    return dev->outputs.count;
}

av_out_t *av_device_get_out(av_device_t *dev, int n) {
    ASSERT(n >= 0 && n < dev->outputs.count);
    return dev->outputs.data[n];
}

static void delete_sreg(av_device_t *dev, av_out_sreg_t *sreg) {
    for(int i = 0; i < dev->sregs.count; ++i) {
        if(dev->sregs.data[i] != sreg)
            continue;
        sreg_buf_remove(&dev->sregs, i);
        return;
    }
}

static void delete_pwm(av_device_t *dev, av_out_pwm_t *pwm) {
    for(int i = 0; i < dev->pwms.count; ++i) {
        if(dev->pwms.data[i] != pwm)
            continue;
        pwm_buf_remove(&dev->pwms, i);
        return;
    }
}

void av_device_delete_out(av_device_t *dev, int idx) {
    ASSERT(idx < dev->outputs.count);
    av_out_t *binding = dev->outputs.data[idx];
    switch(binding->type) {
    case AV_OUT_SHIFT_REG:
        delete_sreg(dev, (av_out_sreg_t *)binding);
        break;
    case AV_OUT_PWM:
        delete_pwm(dev, (av_out_pwm_t *)binding);
        break;
    }
    output_buf_remove(&dev->outputs, idx);
    free(binding);
}

static void init_binding(void *ptr, av_out_type_t type, size_t size) {
    av_out_t *out = ptr;
    memset(ptr, 0, size);
    out->type = type;
    out->id = 0;
}

av_out_sreg_t *av_device_add_out_sreg(av_device_t *dev) {
    av_out_sreg_t *sreg = safe_calloc(1, sizeof(*sreg));
    init_binding(sreg, AV_OUT_SHIFT_REG, sizeof(*sreg));
    sreg_buf_write(&dev->sregs, sreg);
    output_buf_write(&dev->outputs, (av_out_t *)sreg);
    for(int i = 0; i < AV_SREG_MAX_PINS; ++i) {
        av_dref_init(&sreg->pins[i].dref);
        sreg->pins[i].cmp_op = AV_OP_EQ;
        sreg->pins[i].cmp_val = 0.f;
        sreg->pins[i].last_out = -1;
    }
    return sreg;
}

av_out_pwm_t *av_device_add_out_pwm(av_device_t *dev) {
    av_out_pwm_t *pwm = safe_calloc(1, sizeof(*pwm));
    init_binding(pwm, AV_OUT_PWM, sizeof(*pwm));
    pwm_buf_write(&dev->pwms, pwm);
    output_buf_write(&dev->outputs, (av_out_t *)pwm);
    
    av_dref_init(&pwm->dref);
    pwm->mod_op = AV_OP_MULT;
    pwm->mod_val = 1.f;
    pwm->last_out = -1;
    return pwm;
}

av_out_sreg_t *av_device_add_out_sreg_id(av_device_t *dev, int id) {
    for(int i = 0; i < dev->sregs.count; ++i) {
        if(dev->sregs.data[i]->base.id == id)
            return dev->sregs.data[i];
    }
    av_out_sreg_t *sreg = av_device_add_out_sreg(dev);
    sreg->base.id = id;
    return sreg;
}

av_out_pwm_t *av_device_add_out_pwm_id(av_device_t *dev, int id) {
    for(int i = 0; i < dev->pwms.count; ++i) {
        if(dev->pwms.data[i]->base.id == id)
            return dev->pwms.data[i];
    }
    av_out_pwm_t *pwm = av_device_add_out_pwm(dev);
    pwm->base.id = id;
    return pwm;
}

// MARK: - Update Logic

static void commit_cmd(cmd_mgr_t *mgr, serial_t *serial) {
    char buf[128];
    cmd_mgr_send_cmd_commit(mgr);
    size_t len = cmd_mgr_get_output(mgr, buf, sizeof(buf));
    serial_write(serial, buf, len);
}

bool resolve_dref(av_dref_t *dref) {
    if(!dref->has_changed)
        return dref->has_resolved;
    XPLMDataRef ref = XPLMFindDataRef(dref->path);
    if(ref == NULL)
        goto errout;
    
    XPLMDataTypeID type_info = XPLMGetDataRefTypes(ref);
    static const XPLMDataTypeID ok_types = (xplmType_Double | xplmType_Float | xplmType_Int);
    if((type_info & ok_types) == 0)
        goto errout; 
    if(type_info & xplmType_Int)
        dref->type = AV_TYPE_INT;
    else if(type_info & xplmType_Float)
        dref->type = AV_TYPE_FLOAT;
    else if(type_info & xplmType_Double)
        dref->type = AV_TYPE_DOUBLE;
    else
        VERIFY(false);
    
    dref->ref = ref;
    dref->has_resolved = true;
    dref->has_changed = false;
    return true;
errout:
    dref->ref = NULL;
    dref->has_changed = false;
    dref->has_resolved = false;
    dref->type = AV_TYPE_INVALID;
    return false;
}

static bool update_sreg_pin(av_out_sreg_pin_t *pin) {
    if(!resolve_dref(&pin->dref))
        return false;
    
    int64_t value_int = 0;
    float value = NAN;
    switch(pin->dref.type) {
    case AV_TYPE_INVALID:
        return false;
    case AV_TYPE_FLOAT:
        value = XPLMGetDataf(pin->dref.ref);
        break;
    case AV_TYPE_DOUBLE:
        value = XPLMGetDatad(pin->dref.ref);
        break;
    case AV_TYPE_INT:
        value_int = XPLMGetDatai(pin->dref.ref);
        value = (float)value_int;
        break;
    }
    
    if(isnan(value))
        return false;
    
    bool output;
    switch(pin->cmp_op) {
    case AV_OP_NEQ:
        output = fabs(value - pin->cmp_val) > 0.001;
        break;
    case AV_OP_EQ:
        output = fabs(value - pin->cmp_val) < 0.001;
        break;
    case AV_OP_LT:
        output = (value < pin->cmp_val);
        break;
    case AV_OP_LTEQ:
        output = (value < pin->cmp_val);
        break;
    case AV_OP_GT:
        output = (value > pin->cmp_val);
        break;
    case AV_OP_GTEQ:
        output = (value >= pin->cmp_val);
        break;
    case AV_OP_TEST:
        output = (value_int & (int64_t)pin->cmp_val) != 0;
        break;
    }
    
    if(output == pin->last_out)
        return false;
    
    pin->last_out = output;
    return true;
}

void update_sreg(av_out_sreg_t *sreg, av_device_t *dev) {
    UNUSED(sreg);
    UNUSED(dev);
    
    char cmd_on[AV_SREG_MAX_PINS * 4] = "";
    int cmd_on_len = 0;
    
    char cmd_off[AV_SREG_MAX_PINS * 4] = "";
    int cmd_off_len = 0;
    
    for(int i = 0; i < AV_SREG_MAX_PINS; ++i) {
        if(!update_sreg_pin(&sreg->pins[i]))
            continue;
        
        if(sreg->pins[i].last_out) {
            if(cmd_on_len > 0)
                cmd_on[cmd_on_len++] = '|';
            cmd_on_len += snprintf(cmd_on + cmd_on_len, sizeof(cmd_on) - cmd_on_len, "%d", i);
        } else {
            if(cmd_off_len > 0)
                cmd_off[cmd_off_len++] = '|';
            cmd_off_len += snprintf(cmd_off + cmd_off_len, sizeof(cmd_off) - cmd_off_len, "%d", i);
        }
    }
    
    if(cmd_on_len > 0) {
        cmd_mgr_send_cmd_start(&dev->mgr, kSetShiftRegisterPins);
        cmd_mgr_send_arg_int(&dev->mgr, sreg->base.id);
        cmd_mgr_send_arg_cstr(&dev->mgr, cmd_on);
        cmd_mgr_send_arg_int(&dev->mgr, 1);
        commit_cmd(&dev->mgr, dev->serial);
    }
    
    if(cmd_off_len > 0) {
        cmd_mgr_send_cmd_start(&dev->mgr, kSetShiftRegisterPins);
        cmd_mgr_send_arg_int(&dev->mgr, sreg->base.id);
        cmd_mgr_send_arg_cstr(&dev->mgr, cmd_off);
        cmd_mgr_send_arg_int(&dev->mgr, 0);
        commit_cmd(&dev->mgr, dev->serial);
    }
}

void update_pwm(av_out_pwm_t *pwm, av_device_t *dev) {
    if(!resolve_dref(&pwm->dref))
        return;
    
    float value = NAN;
    switch(pwm->dref.type) {
    case AV_TYPE_INVALID:
        return;
    case AV_TYPE_FLOAT:
        value = XPLMGetDataf(pwm->dref.ref);
        break;
    case AV_TYPE_DOUBLE:
        value = XPLMGetDatad(pwm->dref.ref);
        break;
    case AV_TYPE_INT:
        value = XPLMGetDatai(pwm->dref.ref);
        break;
    }
    if(isnan(value))
        return;
    
    switch(pwm->mod_op) {
    case AV_OP_PLUS:
        value += pwm->mod_val;
        break;
    case AV_OP_MINUS:
        value -= pwm->mod_val;
        break;
    case AV_OP_MULT:
        value *= pwm->mod_val;
        break;
    }
    
    int pwm_out = clamp(value, 0.f, 1.f) * 254;
    if(pwm_out == pwm->last_out)
        return;
    
    pwm->last_out = pwm_out;
    cmd_mgr_send_cmd_start(&dev->mgr, kSetPin);
    cmd_mgr_send_arg_int(&dev->mgr, pwm->base.id);
    cmd_mgr_send_arg_int(&dev->mgr, pwm_out);
    commit_cmd(&dev->mgr, dev->serial);
}
