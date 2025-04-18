/*===--------------------------------------------------------------------------------------------===
 * main.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <serial/serial.h>
#include "cmd_mgr.h"

static int brightness = 120;
static serial_t *serial = NULL;


static void commit_cmd(cmd_mgr_t *mgr, serial_t *serial) {
    char buf[128];
    size_t len = cmd_mgr_get_output(mgr, buf, sizeof(buf));
    serial_write(serial, buf, len);
}

static void do_light_test(cmd_mgr_t *mgr) {
    for(int i = 0; i < 14; ++i) {
        cmd_mgr_send_cmd_start(mgr, 27);
        cmd_mgr_send_arg_int(mgr, 0);
        cmd_mgr_send_arg_int(mgr, i);
        cmd_mgr_send_arg_int(mgr, 1);
        cmd_mgr_send_cmd_commit(mgr);
        commit_cmd(mgr, serial);
        
        usleep(100e3);
        
        cmd_mgr_send_cmd_start(mgr, 27);
        cmd_mgr_send_arg_int(mgr, 0);
        cmd_mgr_send_arg_int(mgr, i);
        cmd_mgr_send_arg_int(mgr, 0);
        cmd_mgr_send_cmd_commit(mgr);
        commit_cmd(mgr, serial);
    }
}

static void process_info(cmd_mgr_t *mgr) {
    cmd_mgr_get_arg_str(mgr, NULL, 0);
    char name[64], serial_num[64];
    cmd_mgr_get_arg_str(mgr, name, sizeof(name));
    cmd_mgr_get_arg_str(mgr, serial_num, sizeof(serial_num));
    cmd_mgr_get_arg_str(mgr, NULL, 0);
    cmd_mgr_get_arg_str(mgr, NULL, 0);
    
    printf("info\t%s === %s\n", name, serial_num);
}

static void process_mux(cmd_mgr_t *mgr) {
    char name[32];
    cmd_mgr_get_arg_str(mgr, name, sizeof(name));
    
    int16_t button = cmd_mgr_get_arg_int(mgr);
    int16_t value = cmd_mgr_get_arg_int(mgr);
    (void)button;
    (void)value;
    // printf("mux\t%s:%d: %d\n", name, button, value);
    
}

static void process_encoder(cmd_mgr_t *mgr) {
    char name[32];
    cmd_mgr_get_arg_str(mgr, name, sizeof(name));
    
    int16_t value = cmd_mgr_get_arg_int(mgr);
    // printf("enc\t%s: %d\n", name, value);
    
    if(strcmp(name, "EC_ALT") == 0) {
        
        if(value == 0 || value == 1)
            brightness -= 8;
        if(value == 2 || value == 3)
            brightness += 8;
        
        if(brightness < 0)
            brightness = 0;
        if(brightness > 250)
            brightness = 250;
        
        cmd_mgr_send_cmd_start(mgr, 2);
        cmd_mgr_send_arg_int(mgr, 8);
        cmd_mgr_send_arg_int(mgr, brightness);
        cmd_mgr_send_cmd_commit(mgr);
        commit_cmd(mgr, serial);
    }
}

static void process_switch(cmd_mgr_t *mgr) {
    char name[32];
    cmd_mgr_get_arg_str(mgr, name, sizeof(name));
    
    int16_t value = cmd_mgr_get_arg_int(mgr);
    (void)value;
    
    if(strcmp(name, "APR") == 0 && value == 1) {
        do_light_test(mgr);
    }
    
}


int main(int argc, const char **argv) {
    if(argc != 2) {
        fprintf(stderr, "invalid number of arguments\n");
        return -1;
    }
    
    const char *address = argv[1];
    serial = serial_open(address, SERIAL_BAUDS_115200);
    if(serial == NULL) {
        fprintf(stderr, "unable to open serial port\n");
        return -1;
    }
    
    
    serial_info_t devices[32] = {};
    int device_count = serial_list_devices(devices, 32);
    for(int i = 0; i < device_count; ++i) {
        printf("device: %s (%s)\n", devices[i].name, devices[i].address);
    }
    serial_free_list(devices, device_count);
    
    cmd_mgr_t mgr = {};
    cmd_mgr_init(&mgr);

    cmd_mgr_send_cmd_start(&mgr, 2);
    cmd_mgr_send_arg_int(&mgr, 8);
    cmd_mgr_send_arg_int(&mgr, 250);
    cmd_mgr_send_cmd_commit(&mgr);

    cmd_mgr_send_cmd_start(&mgr, 9);
    cmd_mgr_send_cmd_commit(&mgr);
    commit_cmd(&mgr, serial);
    
    while(1) {
        char buf[512];
        int len = serial_read(serial, buf, sizeof(buf));
        if(len < 0) {
            fprintf(stderr, "serial device disconnected\n");
            break;
        }
        if(len == 0)
            continue;
        cmd_mgr_proccess_input(&mgr, buf, len);
        
        
        int16_t cmd = 0;
        if((cmd = cmd_mgr_get_cmd(&mgr)) < 0)
            continue;
        switch(cmd) {
        case 6:
            process_encoder(&mgr);
            break;
        case 7:
            process_switch(&mgr);
            break;
        case 10:
            process_info(&mgr);
            break;
        case 28:
            break;
        case 30:
            process_mux(&mgr);
            break;
        }
    }
    
    cmd_mgr_fini(&mgr);
}
