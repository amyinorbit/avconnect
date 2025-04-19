/*===--------------------------------------------------------------------------------------------===
 * device.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "device.h"
#include "cmd_ids.h"
#include "utils/cmd_mgr.h"
#include "utils/buffers.h"
#include <serial/serial.h>
#include <acfutils/helpers.h>

#define MAX_BINDINGS        (128)
#define MAX_TYPE_BINDINGS   (32)

DECLARE_BUFFER(encoder, av_in_encoder_t, MAX_TYPE_BINDINGS);
DECLARE_BUFFER(button, av_in_button_t, MAX_TYPE_BINDINGS);
DECLARE_BUFFER(mux, av_in_mux_t, MAX_TYPE_BINDINGS);
DECLARE_BUFFER(input, av_in_t *, MAX_BINDINGS);

DEFINE_BUFFER(encoder, av_in_encoder_t, MAX_TYPE_BINDINGS);
DEFINE_BUFFER(button, av_in_button_t, MAX_TYPE_BINDINGS);
DEFINE_BUFFER(mux, av_in_mux_t, MAX_TYPE_BINDINGS);
DEFINE_BUFFER(input, av_in_t *, MAX_BINDINGS);

#define MAX_CMD_CB      (34)

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
    mux_buf_t           mux_pins;
    
    cmd_cb_t            callbacks[MAX_CMD_CB];
};

av_device_t *av_device_new(const char *address) {
    av_device_t *dev = safe_calloc(1, sizeof(*dev));
    
    dev->serial = NULL;
    dev->name[0] = '\0';
    dev->serial_no[0] = '\0';
    
    input_buf_init(&dev->inputs);
    encoder_buf_init(&dev->encoders);
    button_buf_init(&dev->buttons);
    mux_buf_init(&dev->mux_pins);
    
    cmd_mgr_init(&dev->mgr);
    
    memset(dev->callbacks, 0, sizeof(dev->callbacks));

    av_device_set_address(dev, address);
    return dev;
}

void av_device_destroy(av_device_t *dev) {
    cmd_mgr_fini(&dev->mgr);
    input_buf_fini(&dev->inputs);
    encoder_buf_fini(&dev->encoders);
    button_buf_fini(&dev->buttons);
    mux_buf_fini(&dev->mux_pins);
    
    free(dev);
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
    lacf_strlcpy(dev->name, "Fetching device name...", sizeof(dev->name));
    dev->serial = serial_open(address, SERIAL_BAUDS_115200);
    
    if(dev->serial == NULL)
        return;
    
    cmd_mgr_send_cmd_start(&dev->mgr, kGetInfo);
    av_device_commit_output(dev);
    
    // Send message to get 
    
}

bool av_device_is_connected(const av_device_t *dev);
bool av_device_try_connect(const av_device_t *dev);

// MARK: - Input Management

int av_device_get_in_count(const av_device_t *dev) {
    return dev->inputs.count;
}

av_in_t *av_device_get_in(av_device_t *dev, int idx) {
    ASSERT(idx < dev->inputs.count);
    return dev->inputs.data[idx];
}

void av_device_delete_in(av_device_t *dev, int idx) {
    ASSERT(idx < dev->inputs.count);
    
    int binding_idx = 0;
    av_in_t *binding = dev->inputs.data[idx];
    switch(binding->type) {
    case AV_IN_ENCODER:
        binding_idx = ((av_in_encoder_t*)binding) - dev->encoders.data;
        encoder_buf_remove(&dev->encoders, binding_idx);
        break;
    case AV_IN_BUTTON:
        binding_idx = ((av_in_button_t*)binding) - dev->buttons.data;
        button_buf_remove(&dev->buttons, binding_idx);
        break;
    case AV_IN_MUX:
        binding_idx = ((av_in_mux_t*)binding) - dev->mux_pins.data;
        mux_buf_remove(&dev->mux_pins, binding_idx);
        break;
    }
    input_buf_remove(&dev->inputs, idx);
}

static void init_binding(void *ptr, av_in_type_t type, size_t size, int num) {
    av_in_t *in = ptr;
    memset(ptr, 0, size);
    
    const char *type_str = "";
    switch(type) {
        case AV_IN_ENCODER:     type_str = "Encoder";       break;
        case AV_IN_BUTTON:      type_str = "Button";        break;
        case AV_IN_MUX:         type_str = "Multiplexer";   break;
    }
    
    in->type = type;
    in->name[0] = '\0';
    snprintf(in->comment, sizeof(in->comment), "%s #%d", type_str, num);
}

av_in_encoder_t *av_device_add_in_encoder(av_device_t *dev) {
    if(dev->inputs.count >= MAX_BINDINGS)
        return NULL;
    if(dev->encoders.count >= MAX_TYPE_BINDINGS)
        return NULL;
    av_in_encoder_t *enc = encoder_buf_add(&dev->encoders);
    init_binding(enc, AV_IN_ENCODER, sizeof(*enc), dev->encoders.count);
    return enc;
}

av_in_button_t *av_device_add_in_button(av_device_t *dev) {
    if(dev->inputs.count >= MAX_BINDINGS)
        return NULL;
    if(dev->buttons.count >= MAX_TYPE_BINDINGS)
        return NULL;
    av_in_button_t *button = button_buf_add(&dev->buttons);
    init_binding(button, AV_IN_BUTTON, sizeof(*button), dev->buttons.count);
    return button;
}

av_in_mux_t *av_device_add_in_mux(av_device_t *dev) {
    if(dev->inputs.count >= MAX_BINDINGS)
        return NULL;
    if(dev->mux_pins.count >= MAX_TYPE_BINDINGS)
        return NULL;
    av_in_mux_t *mux = mux_buf_add(&dev->mux_pins);
    init_binding(mux, AV_IN_MUX, sizeof(*mux), dev->mux_pins.count);
    return mux;
}

// MARK: - Device update

void av_device_update(av_device_t *dev) {
    if(dev->serial == NULL)
        return;
    
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
    
    // Feed data to the command manager to actually process stuff
    if(len > 0) {
        int16_t cmd = 0;
        if((cmd = cmd_mgr_get_cmd(&dev->mgr)) >= 0) {
            if(cmd < MAX_CMD_CB && dev->callbacks[cmd] != NULL) {
                dev->callbacks[cmd](dev);
            } else {
                cmd_mgr_skip_cmd(&dev->mgr);
            }
        }
    }
}

