/*===--------------------------------------------------------------------------------------------===
 * device_input.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "device_impl.h"


static av_in_encoder_t *find_encoder(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->encoders.count; ++i) {
        if(strcmp(dev->encoders.data[i]->base.name, name) == 0)
            return dev->encoders.data[i];
    }
    return NULL;
}

static av_in_button_t *find_button(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->buttons.count; ++i) {
        if(strcmp(dev->buttons.data[i]->base.name, name) == 0)
            return dev->buttons.data[i];
    }
    return NULL;
}

static av_in_mux_t *find_mux(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->muxes.count; ++i) {
        if(strcmp(dev->muxes.data[i]->base.name, name) == 0)
            return dev->muxes.data[i];
    }
    return NULL;
}


void callback_encoder(av_device_t *dev) {
    enum {
        EV_DOWN_FAST    = 0,
        EV_DOWN         = 1,
        EV_UP           = 2,
        EV_UP_FAST      = 3
    };
    
    static char name[64];
    if(cmd_mgr_get_arg_str(&dev->mgr, name, sizeof(name)) <= 0)
        return;
    av_in_encoder_t *encoder = find_encoder(dev, name);
    if(encoder == NULL)
        return;
    
    int16_t ev = cmd_mgr_get_arg_int(&dev->mgr);
    if(ev == INT16_MAX)
        return;
    
    switch(ev) {
    case EV_DOWN_FAST:
    case EV_DOWN:
        av_cmd_once(&encoder->cmd_dn);
        break;
    case EV_UP_FAST:
    case EV_UP:
        av_cmd_once(&encoder->cmd_up);
        break;
    }
}

void callback_button(av_device_t *dev) {
    static char name[64];
    if(cmd_mgr_get_arg_str(&dev->mgr, name, sizeof(name)) <= 0)
        return;
    av_in_button_t *button = find_button(dev, name);
    if(button == NULL)
        return;
    if(button->cmd.ref == NULL)
        return;
    
    int16_t ev = cmd_mgr_get_arg_int(&dev->mgr);
    if(ev < 0 || ev > 1)
        return;
    
    if(ev == 1)
        av_cmd_begin(&button->cmd);
    else
        av_cmd_end(&button->cmd);
}

void callback_mux(av_device_t *dev) {
    static char name[64];
    if(cmd_mgr_get_arg_str(&dev->mgr, name, sizeof(name)) <= 0)
        return;
    av_in_mux_t *mux = find_mux(dev, name);
    if(mux == NULL)
        return;
    
    int16_t pin = cmd_mgr_get_arg_int(&dev->mgr);
    if(pin >= AV_MUX_MAX_PINS)
        return;
    int16_t ev = cmd_mgr_get_arg_int(&dev->mgr);
    if(ev < 0 || ev > 1)
        return;
    
    if(mux->cmd[pin].ref == NULL)
        return;
    
    if(ev == 1)
        av_cmd_begin(&mux->cmd[pin]);
    else
        av_cmd_end(&mux->cmd[pin]);
}

void callback_info(av_device_t *dev) {
    
    char buf_ignore[16];
    
    cmd_mgr_get_arg_str(&dev->mgr, buf_ignore, sizeof(buf_ignore)); // ignore <>
    cmd_mgr_get_arg_str(&dev->mgr, dev->name, sizeof(dev->name));
    cmd_mgr_get_arg_str(&dev->mgr, dev->serial_no, sizeof(dev->serial_no));
    cmd_mgr_skip_cmd(&dev->mgr); // Ignore <> and <>
}

void callback_config(av_device_t *dev) {
    UNUSED(dev);
    
}
