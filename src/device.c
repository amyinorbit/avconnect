/*===--------------------------------------------------------------------------------------------===
 * device.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "device_impl.h"

DEFINE_BUFFER(encoder, av_in_encoder_t *);
DEFINE_BUFFER(button, av_in_button_t *);
DEFINE_BUFFER(mux, av_in_mux_t *);
DEFINE_BUFFER(input, av_in_t *);

DEFINE_BUFFER(output, av_out_t *);
DEFINE_BUFFER(sreg, av_out_sreg_t *);
DEFINE_BUFFER(pwm, av_out_pwm_t *);

av_device_t *av_device_new() {
    av_device_t *dev = safe_calloc(1, sizeof(*dev));
    
    dev->serial = NULL;
    dev->name[0] = '\0';
    dev->serial_no[0] = '\0';
    dev->diag[0] = '\0';
    
    input_buf_init(&dev->inputs);
    encoder_buf_init(&dev->encoders);
    button_buf_init(&dev->buttons);
    mux_buf_init(&dev->muxes);
    
    output_buf_init(&dev->outputs);
    sreg_buf_init(&dev->sregs);
    pwm_buf_init(&dev->pwms);
    
    cmd_mgr_init(&dev->mgr);
    memset(dev->callbacks, 0, sizeof(dev->callbacks));
    

    dev->callbacks[kEncoderChange] = callback_encoder;
    dev->callbacks[kButtonChange] = callback_button;
    dev->callbacks[kInfo] = callback_info;
    dev->callbacks[kDigInMuxChange] = callback_mux;
    return dev;
}

static void end_commands(av_device_t *dev) {
    for(int i = 0; i < dev->encoders.count; ++i) {
        av_in_encoder_t *encoder = dev->encoders.data[i];
        av_cmd_end(&encoder->cmd_dn);
        av_cmd_end(&encoder->cmd_up);
    }
    for(int i = 0; i < dev->buttons.count; ++i) {
        av_in_button_t *button = dev->buttons.data[i];
        av_cmd_end(&button->cmd);
    }
    for(int i = 0; i < dev->muxes.count; ++i) {
        av_in_mux_t *mux = dev->muxes.data[i];
        for(int j = 0; j < AV_MUX_MAX_PINS; ++j) {
            av_cmd_end(&mux->cmd[j]);
        }
    }
}

void av_device_destroy(av_device_t *dev) {
    av_device_out_reset(dev);
    end_commands(dev);
    
    for(int i = 0; i < dev->inputs.count; ++i)
        free(dev->inputs.data[i]);
    for(int i = 0; i < dev->outputs.count; ++i)
        free(dev->outputs.data[i]);
    
    cmd_mgr_fini(&dev->mgr);
    input_buf_fini(&dev->inputs);
    encoder_buf_fini(&dev->encoders);
    button_buf_fini(&dev->buttons);
    mux_buf_fini(&dev->muxes);
    
    output_buf_fini(&dev->outputs);
    sreg_buf_fini(&dev->sregs);
    pwm_buf_fini(&dev->pwms);
    
    free(dev);
}

const char *av_device_get_name(const av_device_t *dev) {
    return strlen(dev->name) > 0 ? dev->name : "<no name>";
}

static void av_device_commit_output(av_device_t *dev) {
    if(dev->serial == NULL)
        return;
    cmd_mgr_send_cmd_commit(&dev->mgr);
    char buf[64];
    int len = cmd_mgr_get_output(&dev->mgr, buf, sizeof(buf));
    if(len == 0)
        return;
    serial_write(dev->serial, buf, len);
}

void av_device_set_address(av_device_t *dev, const char *address) {
    if(dev->serial != NULL) {
        serial_close(dev->serial);
        dev->serial = NULL;
        // TODO: Send some kind of "reset to default state message maybe"
    }
    cmd_mgr_fini(&dev->mgr);
    cmd_mgr_init(&dev->mgr);

    lacf_strlcpy(dev->address, address, sizeof(dev->address));
    lacf_strlcpy(dev->name, "<no name received>", sizeof(dev->name));
    av_device_try_connect(dev);
}

const char *av_device_get_address(const av_device_t *dev) {
    return dev->address;
}

bool av_device_is_connected(const av_device_t *dev) {
    return dev->serial != NULL;
}

bool av_device_try_connect(av_device_t *dev) {
    if(dev->serial != NULL)
        return true;
    
    dev->serial = serial_open(dev->address, SERIAL_BAUDS_115200);
    
    if(dev->serial == NULL)
        return false;
    
    cmd_mgr_send_cmd_start(&dev->mgr, kGetInfo);
    av_device_commit_output(dev);
    return true;
}

// MARK: - Device update

void av_device_update(av_device_t *dev) {
    if(dev->serial == NULL)
        return;
    
    // Update command bindings if necessary
    for(int i = 0; i < dev->encoders.count; ++i) {
        update_encoder(dev->encoders.data[i]);
    }
    for(int i = 0; i < dev->buttons.count; ++i) {
        update_button(dev->buttons.data[i]);
    }
    for(int i = 0; i < dev->muxes.count; ++i) {
        update_mux(dev->muxes.data[i]);
    }
    
    for(int i = 0; i < dev->sregs.count; ++i) {
        update_sreg(dev->sregs.data[i], dev);
    }
    for(int i = 0; i < dev->pwms.count; ++i) {
        update_pwm(dev->pwms.data[i], dev);
    }
    
    // Get data from the serial connection
    char buf[512];
    int len = serial_read(dev->serial, buf, sizeof(buf));
    
    if(len < 0) {
        // Device has been lost. We need to do some stuff here
        snprintf(dev->diag, sizeof(dev->diag), "connection lost");
        serial_close(dev->serial);
        dev->serial = NULL;
        return;
    }
    
    cmd_mgr_proccess_input(&dev->mgr, buf, len);
    
    // Feed data to the command manager to actually process stuff
    int16_t cmd = 0;
    if((cmd = cmd_mgr_get_cmd(&dev->mgr)) >= 0) {
        if(cmd < MAX_CMD_CB && dev->callbacks[cmd] != NULL) {
            dev->callbacks[cmd](dev);
        }
        cmd_mgr_skip_cmd(&dev->mgr);
    }
}

