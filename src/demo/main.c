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

static void process_mux() {
    char name[32];
    cmd_mgr_get_arg_str(name, sizeof(name));
    
    int16_t button = cmd_mgr_get_arg_int();
    int16_t value = cmd_mgr_get_arg_int();
    printf("mux\t%s:%d: %d\n", name, button, value);
    
}

static void process_encoder() {
    char name[32];
    cmd_mgr_get_arg_str(name, sizeof(name));
    
    int16_t value = cmd_mgr_get_arg_int();
    printf("enc\t%s: %d\n", name, value);
}

static void process_switch() {
    char name[32];
    cmd_mgr_get_arg_str(name, sizeof(name));
    
    int16_t value = cmd_mgr_get_arg_int();
    printf("but\t%s: %d\n", name, value);
}

int main(int argc, const char **argv) {
    if(argc != 2) {
        fprintf(stderr, "invalid number of arguments\n");
        return -1;
    }
    
    const char *address = argv[1];
    serial_t *serial = serial_open(address, SERIAL_BAUDS_115200);
    if(serial == NULL) {
        fprintf(stderr, "unable to open serial port\n");
        return -1;
    }
    
    cmd_mgr_init();
    
    for(int i = 0; i < 14; ++i) {
        char buf[128];
        
        cmd_mgr_send_cmd_start(27);
        cmd_mgr_send_arg_int(0);
        cmd_mgr_send_arg_int(i);
        cmd_mgr_send_arg_int(1);
        cmd_mgr_send_cmd_commit();
        
        size_t len = cmd_mgr_get_output(buf, sizeof(buf));
        serial_write(serial, buf, len);
        usleep(100e3);
        
        cmd_mgr_send_cmd_start(27);
        cmd_mgr_send_arg_int(0);
        cmd_mgr_send_arg_int(i);
        cmd_mgr_send_arg_int(0);
        cmd_mgr_send_cmd_commit();
        
        len = cmd_mgr_get_output(buf, sizeof(buf));
        serial_write(serial, buf, len);
    }
    

    {
        cmd_mgr_send_cmd_start(2);
        cmd_mgr_send_arg_int(8);
        cmd_mgr_send_arg_int(250);
        cmd_mgr_send_cmd_commit();
    
        cmd_mgr_send_cmd_start(12);
        cmd_mgr_send_cmd_commit();

        char buf[64];
        size_t len = cmd_mgr_get_output(buf, sizeof(buf));
        serial_write(serial, buf, len);
    }
    
    while(1) {
        char buf[512];
        size_t len = serial_read(serial, buf, sizeof(buf));
        if(len == 0)
            continue;
        cmd_mgr_proccess_input(buf, len);
        
        
        int16_t cmd = 0;
        if((cmd = cmd_mgr_get_cmd()) < 0)
            continue;
        switch(cmd) {
        case 6:
            process_encoder();
            break;
        case 7:
            process_switch();
            break;
        case 10:
            break;
        case 28:
            break;
        case 30:
            process_mux();
            break;
        }
    }
    
    cmd_mgr_fini();
}
