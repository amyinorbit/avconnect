/*===--------------------------------------------------------------------------------------------===
 * device.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _MAPPING_H_
#define _MAPPING_H_

#include "bindings/inputs.h"
#include "bindings/outputs.h"
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct av_device_t av_device_t;

av_device_t *av_device_new();
void av_device_destroy(av_device_t *dev);

const char *av_device_get_name(const av_device_t *dev);
void av_device_set_address(av_device_t *dev, const char *address);
const char *av_device_get_address(const av_device_t *dev);
bool av_device_is_connected(const av_device_t *dev);
bool av_device_try_connect(av_device_t *dev);

int av_device_get_in_count(const av_device_t *dev);
av_in_t *av_device_get_in(av_device_t *dev, int idx);
void av_device_delete_in(av_device_t *dev, int idx);

av_in_encoder_t *av_device_add_in_encoder(av_device_t *dev);
av_in_button_t *av_device_add_in_button(av_device_t *dev);
av_in_mux_t *av_device_add_in_mux(av_device_t *dev);

av_in_encoder_t *av_device_add_in_encoder_str(av_device_t *dev, const char *name);
av_in_button_t *av_device_add_in_button_str(av_device_t *dev, const char *name);
av_in_mux_t *av_device_add_in_mux_str(av_device_t *dev, const char *name);

void av_device_out_reset(av_device_t *dev);
int av_device_get_out_count(const av_device_t *dev);
av_out_t *av_device_get_out(av_device_t *dev, int idx);
void av_device_delete_out(av_device_t *dev, int idx);

av_out_sreg_t *av_device_add_out_sreg(av_device_t *dev);
av_out_pwm_t *av_device_add_out_pwm(av_device_t *dev);

av_out_sreg_t *av_device_add_out_sreg_id(av_device_t *dev, int id);
av_out_pwm_t *av_device_add_out_pwm_id(av_device_t *dev, int id);

void av_device_update(av_device_t *dev);

void av_device_write(const av_device_t *dev, FILE *out);

#ifdef __cplusplus
}
#endif


#endif /* ifndef _MAPPING_H_ */
