/*===--------------------------------------------------------------------------------------------===
 * device_impl.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _DEVICE_IMPL_H_
#define _DEVICE_IMPL_H_

#include "device.h"
#include "cmd_ids.h"
#include "utils/cmd_mgr.h"
#include "utils/buffers.h"
#include <serial/serial.h>
#include <acfutils/helpers.h>

#define MAX_CMD_CB      (34)

DECLARE_BUFFER(encoder, av_in_encoder_t *);
DECLARE_BUFFER(button, av_in_button_t *);
DECLARE_BUFFER(mux, av_in_mux_t *);
DECLARE_BUFFER(input, av_in_t *);

typedef void (*cmd_cb_t)(av_device_t *dev);

struct av_device_t {
    char                address[128];
    char                name[128];
    char                serial_no[128];
    char                diag[128];
    
    serial_t            *serial;
    cmd_mgr_t           mgr;
    
    input_buf_t         inputs;
    encoder_buf_t       encoders;
    button_buf_t        buttons;
    mux_buf_t           muxes;
    
    cmd_cb_t            callbacks[MAX_CMD_CB];
};


void callback_encoder(av_device_t *dev);
void callback_button(av_device_t *dev);
void callback_mux(av_device_t *dev);
void callback_info(av_device_t *dev);

#endif /* ifndef _DEVICE_IMPL_H_ */
