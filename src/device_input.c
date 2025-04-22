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
        if(strcmp(dev->encoders.data[i].base.name, name) == 0)
            return &dev->encoders.data[i];
    }
    return NULL;
}

static av_in_button_t *find_button(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->buttons.count; ++i) {
        if(strcmp(dev->buttons.data[i].base.name, name) == 0)
            return &dev->buttons.data[i];
    }
    return NULL;
}

static av_in_mux_t *find_mux(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->muxes.count; ++i) {
        if(strcmp(dev->muxes.data[i].base.name, name) == 0)
            return &dev->muxes.data[i];
    }
    return NULL;
}


void callback_encoder(av_device_t *dev) {
    static char name[64];
    if(cmd_mgr_get_arg_str(&dev->mgr, name, sizeof(name)) <= 0)
        return;
    av_in_encoder_t *encoder = find_encoder(dev, name);
    if(encoder == NULL)
        return;
    
    int16_t ev = cmd_mgr_get_arg_int(&dev->mgr);
    if(ev == INT16_MAX)
        return;
    
    if(ev <= 1 && encoder->cmd_dn.ref != NULL)
        XPLMCommandOnce(encoder->cmd_dn.ref);
    if(ev >= 2 && encoder->cmd_up.ref != NULL)
        XPLMCommandOnce(encoder->cmd_up.ref);
}

void callback_button(av_device_t *dev) {
    UNUSED(dev);
    UNUSED(find_button);
}

void callback_mux(av_device_t *dev) {
    UNUSED(dev);
    UNUSED(find_mux);
}

void callback_info(av_device_t *dev) {
    /*
    cmd_mgr_get_arg_str(mgr, NULL, 0);
    char name[64], serial_num[64];
    cmd_mgr_get_arg_str(mgr, name, sizeof(name));
    cmd_mgr_get_arg_str(mgr, serial_num, sizeof(serial_num));
    cmd_mgr_get_arg_str(mgr, NULL, 0);
    cmd_mgr_get_arg_str(mgr, NULL, 0);
    
    printf("info\t%s === %s\n", name, serial_num);
    */
    // char                name[128];
    // char                serial_no[128];
    
    cmd_mgr_get_arg_str(&dev->mgr, NULL, 0); // ignore <>
    cmd_mgr_get_arg_str(&dev->mgr, dev->name, sizeof(dev->name));
    cmd_mgr_get_arg_str(&dev->mgr, dev->serial_no, sizeof(dev->serial_no));
    cmd_mgr_skip_cmd(&dev->mgr); // Ignore <> and <>
}
