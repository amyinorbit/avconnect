/*===--------------------------------------------------------------------------------------------===
 * avconnect_cfg.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "avconnect.h"
#include <toml.h>
#include <acfutils/helpers.h>

#define CHECK(dat, err) if(!dat.ok) { logMsg(err); goto out; }

static void parse_encoder(av_device_t *dev, toml_table_t *cencoder) {
    if(cencoder == NULL) {
        logMsg("encoder mapping must be a table");
        return;
    }
    
    toml_datum_t name = toml_string_in(cencoder, "name");
    toml_datum_t cmd_up = toml_string_in(cencoder, "command_up");
    toml_datum_t cmd_dn = toml_string_in(cencoder, "command_down");
    
    CHECK(name, "missing encoder name");
    CHECK(cmd_up, "missing button up command");
    CHECK(cmd_dn, "missing button down command");
    
    av_in_encoder_t *encoder = av_device_add_in_encoder_str(dev, name.u.s);
    lacf_strlcpy(encoder->cmd_dn.path, cmd_dn.u.s, sizeof(encoder->cmd_dn.path));
    lacf_strlcpy(encoder->cmd_up.path, cmd_up.u.s, sizeof(encoder->cmd_up.path));
    
    encoder->cmd_dn.has_changed = true;
    encoder->cmd_up.has_changed = true;
out:
    if(name.ok) free(name.u.s);
    if(cmd_up.ok) free(cmd_up.u.s);
    if(cmd_dn.ok) free(cmd_dn.u.s);
}

static void parse_button(av_device_t *dev, toml_table_t *cbutton) {
    if(cbutton == NULL) {
        logMsg("button mapping must be a table");
        return;
    }
    
    toml_datum_t name = toml_string_in(cbutton, "name");
    toml_datum_t cmd = toml_string_in(cbutton, "command");
    
    CHECK(name, "missing button name");
    CHECK(cmd, "missing button command");
    
    av_in_button_t *button = av_device_add_in_button_str(dev, name.u.s);
    lacf_strlcpy(button->cmd.path, cmd.u.s, sizeof(button->cmd.path));
    
    button->cmd.has_changed = true;
out:
    if(name.ok) free(name.u.s);
    if(cmd.ok) free(cmd.u.s);
}

static void parse_mux(av_device_t *dev, toml_table_t *cmux) {
    if(cmux == NULL) {
        logMsg("multiplexer mapping must be a table");
        return;
    }
    
    toml_datum_t name = toml_string_in(cmux, "name");
    toml_datum_t pin = toml_int_in(cmux, "input");
    toml_datum_t cmd = toml_string_in(cmux, "command");
    
    CHECK(name, "missing mulitplexer name");
    CHECK(pin, "missing multiplexer input");
    CHECK(cmd, "missing multiplexer command");
    
    
    int p = pin.u.i;
    if(p >= AV_MUX_MAX_PINS) {
        logMsg("Only 16 inputs allowed per multiplexer");
        goto out;
    }
    
    av_in_mux_t *mux = av_device_add_in_mux_str(dev, name.u.s);
    
    lacf_strlcpy(mux->cmd[p].path, cmd.u.s, sizeof(mux->cmd[p].path));
    mux->cmd[p].has_changed = true;
    
out:
    if(name.ok) free(name.u.s);
    if(cmd.ok) free(cmd.u.s);
}

#define COUNTOF(x) (sizeof(x)/sizeof(*x))

static int find_str_in(const char **array, int n, const char *str) {
    for(int i = 0; i < n; ++i) {
        if(strcmp(array[i], str) == 0)
            return i;
    }
    return -1;
}

static void parse_pwm(av_device_t *dev, toml_table_t *cpwm) {
    if(cpwm == NULL) {
        logMsg("PWM mapping must be a table");
        return;
    }
    
    toml_datum_t id = toml_int_in(cpwm, "pin");
    toml_datum_t dref = toml_string_in(cpwm, "dataref");
    toml_datum_t mod_str = toml_string_in(cpwm, "op");
    toml_datum_t val = toml_double_in(cpwm, "val");
    
    CHECK(id, "missing pwm output pin number");
    CHECK(dref, "missing pwm output dataref");
    CHECK(mod_str, "missing pwm output modifier");
    CHECK(val, "missing pwm output modifier value");
    
    int mod = find_str_in(av_mod_str, COUNTOF(av_mod_str), mod_str.u.s);
    if(mod < 0) {
        logMsg("invalid dataref modifier: %s", mod_str.u.s);
        goto out;
    }
    
    av_out_pwm_t *pwm = av_device_add_out_pwm_id(dev, id.u.i);
    ASSERT(pwm != NULL);
    
    lacf_strlcpy(pwm->dref.path, dref.u.s, sizeof(pwm->dref.path));
    pwm->mod_op = mod;
    pwm->mod_val = val.u.d;
    pwm->last_out = -1;
    
out:
    if(dref.ok) free(dref.u.s);
    if(mod_str.ok) free(mod_str.u.s);
}

static void parse_sreg(av_device_t *dev, toml_table_t *csreg) {
    if(csreg == NULL) {
        logMsg("PWM mapping must be a table");
        return;
    }
    
    toml_datum_t mod = toml_int_in(csreg, "module");
    toml_datum_t pin_n = toml_int_in(csreg, "output");
    toml_datum_t dref = toml_string_in(csreg, "dataref");
    toml_datum_t cmp_str = toml_string_in(csreg, "op");
    toml_datum_t val = toml_double_in(csreg, "val");
    
    CHECK(mod, "missing shift register output name");
    CHECK(pin_n, "missing shift register output pin number");
    CHECK(dref, "missing shift register output dataref");
    CHECK(cmp_str, "missing shift register output modifier");
    CHECK(val, "missing shift register output modifier value");
    
    int cmp = find_str_in(av_cmp_str, COUNTOF(av_cmp_str), cmp_str.u.s);
    if(cmp < 0) {
        logMsg("invalid dataref comparison operator: %s", cmp_str.u.s);
        goto out;
    }
    
    if(pin_n.u.i >= AV_SREG_MAX_PINS) {
        logMsg("invalid shift register pin number: %d", (int)pin_n.u.i);
        goto out;
    }
    
    av_out_sreg_t *sreg = av_device_add_out_sreg_id(dev, mod.u.i);
    ASSERT(sreg != NULL);
    
    av_out_sreg_pin_t *pin = &sreg->pins[pin_n.u.i];
    
    lacf_strlcpy(pin->dref.path, dref.u.s, sizeof(pin->dref.path));
    pin->cmp_op = cmp;
    pin->cmp_val = val.u.d;
    pin->last_out = -1;
    
out:
    if(dref.ok) free(dref.u.s);
    if(cmp_str.ok) free(cmp_str.u.s);
}

void do_read_conf(char *path) {
    FILE* in = fopen(path, "rb");
    toml_table_t *conf = NULL;
    if(in == NULL) {
        logMsg("could not read configuration `%s`: %s", path, strerror(errno));
        goto out;
    }
    
    char error[128];
    conf = toml_parse_file(in, error, sizeof(error));
    if(conf == NULL) {
        logMsg("could not read configuration `%s`: %s", path, error);
        goto out;
    }
    
    toml_array_t *devices = toml_array_in(conf, "device");
    if(devices == NULL) {
        logMsg("no device found");
        goto out;
    }
    
    avconnect_device_delete_all();
    
    int dev_count = toml_array_nelem(devices);
    
    for(int i = 0; i < dev_count; ++i) {
        toml_table_t *cdev = toml_table_at(devices, i);
        ASSERT(cdev != NULL);
        logMsg("found one device!");
        
        toml_datum_t address = toml_string_in(cdev, "port");
        if(!address.ok) {
            logMsg("missing device address");
            goto out;
        }
        
        av_device_t *dev = avconnect_device_add();
        av_device_set_address(dev, address.u.s);
        
        toml_array_t *encoders = toml_array_in(cdev, "in_encoders");
        if(encoders) {
            for(int i = 0; i < toml_array_nelem(encoders); ++i)
                parse_encoder(dev, toml_table_at(encoders, i));
        }
        
        toml_array_t *buttons = toml_array_in(cdev, "in_buttons");
        if(buttons) {
            for(int i = 0; i < toml_array_nelem(buttons); ++i)
                parse_button(dev, toml_table_at(buttons, i));
        }
        
        toml_array_t *muxes = toml_array_in(cdev, "in_multiplexers");
        if(muxes) {
            for(int i = 0; i < toml_array_nelem(muxes); ++i)
                parse_mux(dev, toml_table_at(muxes, i));
        }
        
        toml_array_t *pwms = toml_array_in(cdev, "out_pwms");
        if(pwms) {
            for(int i = 0; i < toml_array_nelem(pwms); ++i)
                parse_pwm(dev, toml_table_at(pwms, i));
        }
        
        toml_array_t *sregs = toml_array_in(cdev, "out_shift_regs");
        if(sregs) {
            for(int i = 0; i < toml_array_nelem(sregs); ++i)
                parse_sreg(dev, toml_table_at(sregs, i));
        }
        
        free(address.u.s);
        av_device_out_reset(dev);
    }
    
out:
    if(conf != NULL)
        toml_free(conf);
    if(in != NULL)
        fclose(in);
    free(path);
}