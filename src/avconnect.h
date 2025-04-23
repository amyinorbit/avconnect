/*===--------------------------------------------------------------------------------------------===
 * avconnect.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _AVCONNECT_H_
#define _AVCONNECT_H_

#include <stdbool.h>
#include "device.h"

#ifdef __cplusplus
extern "C" {
#endif

void avconnect_init();
void avconnect_fini();

void avconnect_conf_check_reload();
void avconnect_conf_save(bool acf_specific);

int avconnect_get_device_count();
av_device_t *avconnect_device_get(int i);
av_device_t *avconnect_device_add();
void avconnect_device_delete(int i);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _AVCONNECT_H_ */
