/*===--------------------------------------------------------------------------------------------===
 * device_cfg.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "device_impl.h"
#include <toml.h>


// MARK: - Device write functions

// static inline void write_comment(FILE *out, const char *comment) {
//     fprintf(out, "# %s\n", comment);
// }
//
// static inline void write_table(FILE *out, const char *name) {
//     fprintf(out, "[%s]\n\n", name);
// }

static inline void write_table_array(FILE *out, const char *array) {
    fprintf(out, "[[%s]]\n\n", array);
}

static inline void write_string(FILE *out, const char *key, const char *value, const char *after) {
    fprintf(out, "%s = \"%s\"%s", key, value, after);
}

static inline void write_int(FILE *out, const char *key, int value, const char *after) {
    fprintf(out, "%s = %d%s", key, value, after);
}

static inline void write_encoder(FILE *out, const av_in_encoder_t *enc) {
    fprintf(out, "  { ");
    write_string(out, "name", enc->base.name, ", ");
    write_string(out, "command_up", enc->cmd_up.path, ", ");
    write_string(out, "command_down", enc->cmd_up.path, " }");
}

static inline void write_button(FILE *out, const av_in_button_t *button) {
    fprintf(out, "  { ");
    write_string(out, "name", button->base.name, ", ");
    write_string(out, "command", button->cmd.path, " }");
}

static inline void write_mux(FILE *out, const av_in_mux_t *mux, bool is_last_mux) {
    for(int i = 0; i < mux->pin_count; ++i) {
        bool is_last_pin = (i == mux->pin_count - 1);
        const char *end_of_entry = (is_last_mux && is_last_pin) ? " }" : " },";
        
        fprintf(out, "  { ");
        write_string(out, "name", mux->base.name, ", ");
        write_int(out, "input", i, ", ");
        write_string(out, "command", mux->cmd[i].path, end_of_entry);
    }
}

void av_device_write(const av_device_t *dev, FILE *out) {
    ASSERT(out != NULL);
    
    write_table_array(out, "device");
    write_string(out, "port", dev->address, "\n");
    write_string(out, "id", dev->serial_no, "\n");
    
    fprintf(out, "in_encoders = [\n");
    for(int i = 0; i < dev->encoders.count; ++i) {
        bool is_last = i == dev->encoders.count-1;
        write_encoder(out, dev->encoders.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
    
    fprintf(out, "in_buttons = [\n");
    for(int i = 0; i < dev->buttons.count; ++i) {
        bool is_last = i == dev->buttons.count-1;
        write_button(out, dev->buttons.data[i]);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
    
    fprintf(out, "in_multiplexers = [\n");
    for(int i = 0; i < dev->muxes.count; ++i) {
        bool is_last = i == dev->muxes.count-1;
        write_mux(out, dev->muxes.data[i], is_last);
        fprintf(out, "%s\n", is_last ? "" : ",");
    }
    fprintf(out, "]\n");
}
