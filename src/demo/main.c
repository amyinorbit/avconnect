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

static int parse_commands(char *cmd_buf, int size) {
    while(size > 0) {
        const char *cmd_end = strchr(cmd_buf, ';');
        if(cmd_end == NULL)
            break;
        
        int cmd_len = 1 + (int)(cmd_end - cmd_buf);
        
        printf("command: %.*s\n", cmd_len, cmd_buf);
        memmove(cmd_buf, cmd_end+1, size - cmd_len);
        size -= cmd_len;
        cmd_buf[size] = '\0';
    }
    return size;
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
    
    static const int cap = 512;
    char cmd_buf[cap+1];
    int cmd_size = 0;
    

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
        int rd = (int)serial_read(serial, cmd_buf + cmd_size, cap - cmd_size);
        if(rd == 0)
            continue;
        
        cmd_size += rd;
        cmd_buf[cmd_size] = '\0';
        
        cmd_size = parse_commands(cmd_buf, cmd_size);
        
    }
    
    cmd_mgr_fini();
}
