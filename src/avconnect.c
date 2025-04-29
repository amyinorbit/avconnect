/*===--------------------------------------------------------------------------------------------===
 * avconnect.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include "avconnect.h"
#include "settings.h"
#include "xplane.h"
#include "utils/buffers.h"
#include <acfutils/helpers.h>
#include <toml.h>
#include <XPLMProcessing.h>


DECLARE_BUFFER(device, av_device_t *);
DEFINE_BUFFER(device, av_device_t *);

static float avconnect_floop(float elapsed, float last_floop, int counter, void *refcon);
void do_read_conf(char *path);


static device_buf_t     devices = {};
static bool             is_inited = false;

void avconnect_init() {
    if(is_inited)
        return;
    
    device_buf_init(&devices);
    settings_init();
    XPLMRegisterFlightLoopCallback(avconnect_floop, -1, NULL);
    is_inited = true;
}

void avconnect_fini() {
    if(!is_inited)
        return;
    is_inited = false;

    XPLMUnregisterFlightLoopCallback(avconnect_floop, NULL);
    settings_fini();
    for(int i = 0; i < devices.count; ++i) {
        av_device_destroy(devices.data[i]);
    }
    device_buf_fini(&devices);
}


void avconnect_conf_check_reload(bool acf_specific) {
    
    if(acf_specific) {
        char *path = mkpathname(get_plane_dir(), "avconnect.toml", NULL);
        if(file_exists(path, NULL)) {
            do_read_conf(path);
            return;
        }
        free(path);
    }
    
    char *path = mkpathname(get_conf_dir(), "avconnect.toml", NULL);
    if(file_exists(path, NULL)) {
        do_read_conf(path);
        return;
    }
    free(path);
}

void avconnect_conf_save(bool acf_specific) {
    UNUSED(acf_specific);
    
    char *path = mkpathname(acf_specific ? get_plane_dir() : get_conf_dir(), "avconnect.toml", NULL);
    FILE *out = fopen(path, "wb");
    if(out == NULL) {
        logMsg("unable to save configuration to `%s`: %s", path, strerror(errno));
        free(path);
        return;
    }
    free(path);
    
    fprintf(out, "# AvConnect configuration\n");
    for(int i = 0; i < devices.count; ++i) {
        av_device_write(devices.data[i], out);
    }
    fclose(out);
}

int avconnect_get_device_count() {
    return devices.count;
}

av_device_t *avconnect_device_add() {
    av_device_t *dev = av_device_new();
    device_buf_write(&devices, dev);
    return dev;
}

void avconnect_device_delete(int i) {
    ASSERT(i >= 0 && i < devices.count);
    av_device_destroy(devices.data[i]);
    device_buf_remove(&devices, i);
}

av_device_t *avconnect_device_get(int i) {
    ASSERT(i >= 0 && i < devices.count);
    return devices.data[i];
}

void avconnect_device_delete_all() {
    for(int i = 0; i < devices.count; ++i) {
        av_device_destroy(devices.data[i]);
    }
    devices.count = 0;
}

float avconnect_floop(float elapsed, float last_floop, int counter, void *refcon) {
    UNUSED(elapsed);
    UNUSED(last_floop);
    UNUSED(counter);
    UNUSED(refcon);
    
    for(int i = 0; i < devices.count; ++i) {
        av_device_update(devices.data[i]);
    }
    
    return -1.f;
}
