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


void callback_encoder(av_device_t *dev) {
    UNUSED(dev);
}

void callback_button(av_device_t *dev) {
    UNUSED(dev);
}

void callback_mux(av_device_t *dev) {
    UNUSED(dev);
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
