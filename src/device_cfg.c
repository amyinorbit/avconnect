/*===--------------------------------------------------------------------------------------------===
 * device_cfg.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "device_impl.h"
#include <toml.h>

// MARK: - Device write functions

// static inline void write_comment(FILE *out, const char *comment) {
//     fprintf(out, "# %s\n", comment);
// }
//
// static inline void write_table(FILE *out, const char *name) {
//     fprintf(out, "[%s]\n\n", name);
// }

static inline void write_table_array(FILE *out, const char *array) {
    fprintf(out, "[[%s]]\n\n", array);
}

static inline void write_string(FILE *out, const char *key, const char *value, const char *after) {
    fprintf(out, "%s = \"%s\"%s", key, value, after);
}

static inline void write_int(FILE *out, const char *key, int value, const char *after) {
    fprintf(out, "%s = %d%s", key, value, after);
}

static inline void write_float(FILE *out, const char *key, float value, const char *after) {
    fprintf(out, "%s = %f%s", key, value, after);
}

static void write_encoder(FILE *out, const av_in_encoder_t *enc) {
    fprintf(out, "    { ");
    write_string(out, "name", enc->base.name, ", ");
    write_string(out, "command_up", enc->cmd_up.path, ", ");
    write_string(out, "command_down", enc->cmd_dn.path, " }");
}

static void write_button(FILE *out, const av_in_button_t *button) {
    fprintf(out, "    { ");
    write_string(out, "name", button->base.name, ", ");
    write_string(out, "command", button->cmd.path, " }");
}

static void write_mux(FILE *out, const av_in_mux_t *mux) {
    bool done_first = false;
    for(int i = 0; i < AV_MUX_MAX_PINS; ++i) {
        if(!strlen(mux->cmd[i].path))
            continue;
        
        if(done_first) {
            done_first = true;
            fprintf(out, "    { ");
        } else {
            fprintf(out, ",\n    {");
        }
        write_string(out, "name", mux->base.name, ", ");
        write_int(out, "input", i, ", ");
        write_string(out, "command", mux->cmd[i].path, " }");
    }
}

static void write_pwm(FILE *out, const av_out_pwm_t *pwm) {
    fprintf(out, "    { ");
    write_int(out, "pin", pwm->base.id, ", ");
    write_string(out, "dataref", pwm->dref.path, " ,");
    write_string(out, "op", av_mod_str[pwm->mod_op], ", ");
    write_float(out, "val", pwm->mod_val, " }");
}

static void write_sreg(FILE *out, const av_out_sreg_t *sreg) {
    bool done_first = false;
    for(int i = 0; i < AV_SREG_MAX_PINS; ++i) {
        const av_out_sreg_pin_t *pin = &sreg->pins[i];
        if(!strlen(pin->dref.path))
            continue;
        if(done_first) {
            done_first = true;
            fprintf(out, "    { ");
        } else {
            fprintf(out, ",\n    {");
        }
        
        write_int(out, "module", sreg->base.id, ", ");
        write_int(out, "output", i, ", ");
        write_string(out, "dataref", pin->dref.path, ", ");
        write_string(out, "op", av_cmp_str[pin->cmp_op], ", ");
        write_float(out, "val", pin->cmp_val, " }");
    }
}

void av_device_write(const av_device_t *dev, FILE *out) {
    ASSERT(out != NULL);
    
    write_table_array(out, "device");
    write_string(out, "port", dev->address, "\n");
    write_string(out, "id", dev->serial_no, "\n");
    
    fprintf(out, "in_encoders = [\n");
    for(int i = 0; i < dev->encoders.count; ++i) {
        bool is_last = i == dev->encoders.count-1;
        write_encoder(out, dev->encoders.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
    
    fprintf(out, "in_buttons = [\n");
    for(int i = 0; i < dev->buttons.count; ++i) {
        bool is_last = i == dev->buttons.count-1;
        write_button(out, dev->buttons.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
    
    fprintf(out, "in_multiplexers = [\n");
    for(int i = 0; i < dev->muxes.count; ++i) {
        bool is_last = i == dev->muxes.count-1;
        write_mux(out, dev->muxes.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
    
    
    fprintf(out, "out_pwms = [\n");
    for(int i = 0; i < dev->pwms.count; ++i) {
        bool is_last = i == dev->pwms.count-1;
        write_pwm(out, dev->pwms.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
    
    fprintf(out, "out_shift_regs = [\n");
    for(int i = 0; i < dev->sregs.count; ++i) {
        bool is_last = i == dev->sregs.count-1;
        write_sreg(out, dev->sregs.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
}
