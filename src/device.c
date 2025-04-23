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
    
    cmd_mgr_init(&dev->mgr);
    memset(dev->callbacks, 0, sizeof(dev->callbacks));
    

    dev->callbacks[kEncoderChange] = callback_encoder;
    dev->callbacks[kButtonChange] = callback_button;
    dev->callbacks[kInfo] = callback_info;
    dev->callbacks[kDigInMuxChange] = callback_mux;
    return dev;
}

void av_device_destroy(av_device_t *dev) {
    
    for(int i = 0; i < dev->encoders.count; ++i)
        free(dev->encoders.data[i]);
    for(int i = 0; i < dev->buttons.count; ++i)
        free(dev->buttons.data[i]);
    for(int i = 0; i < dev->muxes.count; ++i)
        free(dev->muxes.data[i]);
    
    cmd_mgr_fini(&dev->mgr);
    input_buf_fini(&dev->inputs);
    encoder_buf_fini(&dev->encoders);
    button_buf_fini(&dev->buttons);
    mux_buf_fini(&dev->muxes);
    
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
    lacf_strlcpy(dev->name, "Fetching device name...", sizeof(dev->name));
    dev->serial = serial_open(address, SERIAL_BAUDS_115200);
    
    if(dev->serial == NULL)
        return;
    
    cmd_mgr_send_cmd_start(&dev->mgr, kGetInfo);
    av_device_commit_output(dev);
}

const char *av_device_get_address(const av_device_t *dev) {
    return dev->address;
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

static void delete_encoder(av_device_t *dev, av_in_encoder_t *encoder) {
    for(int i = 0; i < dev->encoders.count; ++i) {
        if(dev->encoders.data[i] == encoder) {
            encoder_buf_remove(&dev->encoders, i);
        }
    }
}

static void delete_button(av_device_t *dev, av_in_button_t *button) {
    for(int i = 0; i < dev->buttons.count; ++i) {
        if(dev->buttons.data[i] == button) {
            button_buf_remove(&dev->buttons, i);
        }
    }
}

static void delete_mux(av_device_t *dev, av_in_mux_t *mux) {
    for(int i = 0; i < dev->muxes.count; ++i) {
        if(dev->muxes.data[i] == mux) {
            mux_buf_remove(&dev->muxes, i);
        }
    }
}

void av_device_delete_in(av_device_t *dev, int idx) {
    ASSERT(idx < dev->inputs.count);
    av_in_t *binding = dev->inputs.data[idx];
    switch(binding->type) {
    case AV_IN_ENCODER:
        delete_encoder(dev, (av_in_encoder_t *)binding);
        break;
    case AV_IN_BUTTON:
        delete_button(dev, (av_in_button_t *)binding);
        break;
    case AV_IN_MUX:
        delete_mux(dev, (av_in_mux_t *)binding);
        break;
    }
    input_buf_remove(&dev->inputs, idx);
    free(binding);
}

static void init_binding(void *ptr, av_in_type_t type, size_t size) {
    av_in_t *in = ptr;
    memset(ptr, 0, size);
    in->type = type;
    in->name[0] = '\0';
}


av_in_encoder_t *av_device_add_in_encoder(av_device_t *dev) {
    av_in_encoder_t *enc = safe_calloc(1, sizeof(*enc));
    init_binding(enc, AV_IN_ENCODER, sizeof(*enc));
    encoder_buf_write(&dev->encoders, enc);
    input_buf_write(&dev->inputs, (av_in_t *)enc);
    av_cmd_init(&enc->cmd_dn);
    av_cmd_init(&enc->cmd_up);
    return enc;
}

av_in_button_t *av_device_add_in_button(av_device_t *dev) {
    av_in_button_t *button = safe_calloc(1, sizeof(*button));
    init_binding(button, AV_IN_BUTTON, sizeof(*button));
    button_buf_write(&dev->buttons, button);
    input_buf_write(&dev->inputs, (av_in_t *)button);
    av_cmd_init(&button->cmd);
    return button;
}

av_in_mux_t *av_device_add_in_mux(av_device_t *dev) {
    av_in_mux_t *mux = safe_calloc(1, sizeof(*mux));
    init_binding(mux, AV_IN_MUX, sizeof(*mux));
    mux_buf_write(&dev->muxes, mux);
    input_buf_write(&dev->inputs, (av_in_t *)mux);
    for(int i = 0; i < MUX_MAX_PINS; ++i) {
        av_cmd_init(&mux->cmd[i]);
    }
    return mux;
}

// MARK: - Device update

static bool resolve_cmd(av_cmd_t *cmd) {
    if(!cmd->has_changed)
        return cmd->has_resolved;
    cmd->ref = XPLMFindCommand(cmd->path);
    cmd->has_changed = false;
    return cmd->has_resolved = (cmd->ref != NULL);
}

static void update_encoder(av_in_encoder_t *enc) {
    resolve_cmd(&enc->cmd_dn);
    resolve_cmd(&enc->cmd_up);
}

static void update_button(av_in_button_t *button) {
    resolve_cmd(&button->cmd);
}

static void update_mux(av_in_mux_t *mux) {
    for(int i = 0; i < mux->pin_count; ++i) {
        resolve_cmd(&mux->cmd[i]);
    }
}


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
    
    // Get data from the serial connection
    char buf[512];
    int len = serial_read(dev->serial, buf, sizeof(buf));
    cmd_mgr_proccess_input(&dev->mgr, buf, len);
    
    if(len < 0) {
        // Device has been lost. We need to do some stuff here
        snprintf(dev->diag, sizeof(dev->diag), "connection lost");
        serial_close(dev->serial);
        dev->serial = NULL;
        return;
    }
    
    // Feed data to the command manager to actually process stuff
    int16_t cmd = 0;
    if((cmd = cmd_mgr_get_cmd(&dev->mgr)) >= 0) {
        if(cmd < MAX_CMD_CB && dev->callbacks[cmd] != NULL) {
            dev->callbacks[cmd](dev);
        }
        cmd_mgr_skip_cmd(&dev->mgr);
    }
}

