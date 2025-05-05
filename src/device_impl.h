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
#include <time.h>

#define MAX_CMD_CB      (34)
#define CONFIG_TIMEOUT  (10)

DECLARE_BUFFER(encoder, av_in_encoder_t *);
DECLARE_BUFFER(button, av_in_button_t *);
DECLARE_BUFFER(mux, av_in_mux_t *);
DECLARE_BUFFER(input, av_in_t *);

DECLARE_BUFFER(sreg, av_out_sreg_t *);
DECLARE_BUFFER(pwm, av_out_pwm_t *);
DECLARE_BUFFER(output, av_out_t *);

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
    
    output_buf_t        outputs;
    sreg_buf_t          sregs;
    pwm_buf_t           pwms;
    
    time_t              config_req_time;
    
    cmd_cb_t            callbacks[MAX_CMD_CB];
};


void callback_encoder(av_device_t *dev);
void callback_button(av_device_t *dev);
void callback_mux(av_device_t *dev);
void callback_info(av_device_t *dev);

bool resolve_cmd(av_cmd_t *cmd);
void update_encoder(av_in_encoder_t *enc);
void update_button(av_in_button_t *button);
void update_mux(av_in_mux_t *mux);

bool resolve_dref(av_dref_t *dref);
void update_sreg(av_out_sreg_t *sreg, av_device_t *dev);
void update_pwm(av_out_pwm_t *pwm, av_device_t *dev);

void clear_bindings(av_device_t *dev);
void parse_config(av_device_t *dev, char *cfg);

#endif /* ifndef _DEVICE_IMPL_H_ */
