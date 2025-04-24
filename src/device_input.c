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
            return;
        }
    }
}

static void delete_button(av_device_t *dev, av_in_button_t *button) {
    for(int i = 0; i < dev->buttons.count; ++i) {
        if(dev->buttons.data[i] == button) {
            button_buf_remove(&dev->buttons, i);
            return;
        }
    }
}

static void delete_mux(av_device_t *dev, av_in_mux_t *mux) {
    for(int i = 0; i < dev->muxes.count; ++i) {
        if(dev->muxes.data[i] == mux) {
            mux_buf_remove(&dev->muxes, i);
            return;
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
    for(int i = 0; i < AV_MUX_MAX_PINS; ++i) {
        av_cmd_init(&mux->cmd[i]);
    }
    return mux;
}

av_in_encoder_t *av_device_add_in_encoder_str(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->encoders.count; ++i) {
        if(strcmp(dev->encoders.data[i]->base.name, name) == 0)
            return dev->encoders.data[i];
    }
    av_in_encoder_t *encoder = av_device_add_in_encoder(dev);
    strlcpy(encoder->base.name, name, sizeof(encoder->base.name));
    return encoder;
}

av_in_button_t *av_device_add_in_button_str(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->buttons.count; ++i) {
        if(strcmp(dev->buttons.data[i]->base.name, name) == 0)
            return dev->buttons.data[i];
    }
    av_in_button_t *button = av_device_add_in_button(dev);
    strlcpy(button->base.name, name, sizeof(button->base.name));
    return button;
}

av_in_mux_t *av_device_add_in_mux_str(av_device_t *dev, const char *name) {
    for(int i = 0; i < dev->muxes.count; ++i) {
        if(strcmp(dev->muxes.data[i]->base.name, name) == 0)
            return dev->muxes.data[i];
    }
    av_in_mux_t *mux = av_device_add_in_mux(dev);
    strlcpy(mux->base.name, name, sizeof(mux->base.name));
    return mux;
}

// MARK: - Update Logic

bool resolve_cmd(av_cmd_t *cmd) {
    if(!cmd->has_changed)
        return cmd->has_resolved;
    cmd->ref = XPLMFindCommand(cmd->path);
    cmd->has_changed = false;
    return cmd->has_resolved = (cmd->ref != NULL);
}

void update_encoder(av_in_encoder_t *enc) {
    resolve_cmd(&enc->cmd_dn);
    resolve_cmd(&enc->cmd_up);
}

void update_button(av_in_button_t *button) {
    resolve_cmd(&button->cmd);
}

void update_mux(av_in_mux_t *mux) {
    for(int i = 0; i < AV_MUX_MAX_PINS; ++i) {
        resolve_cmd(&mux->cmd[i]);
    }
}
