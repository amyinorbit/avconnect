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

typedef struct av_device_t av_device_t;

av_device_t *av_device_new(const char *address);
void av_device_destroy(av_device_t *conn);

void av_device_set_address(const char *address);
bool av_device_is_connected();
bool av_device_try_connect();

int av_device_get_in_count(const av_device_t *device);
av_in_t *av_device_get_in(av_device_t *device, int idx);
void av_device_delete_in(av_device_t *device, int idx);

av_in_encoder_t *av_device_add_in_encoder(av_device_t *device);
av_in_button_t *av_device_add_in_button(av_device_t *device);
av_in_mux_t *av_device_add_in_mux(av_device_t *device);

void av_device_update(av_device_t *device);

#endif /* ifndef _MAPPING_H_ */
