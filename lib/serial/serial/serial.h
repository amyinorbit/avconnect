/*===--------------------------------------------------------------------------------------------===
 * uart.h
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct serial_t serial_t;

typedef struct {
    char *address;
    char *name;
} serial_info_t;

typedef enum {
    SERIAL_BAUDS_300,
    SERIAL_BAUDS_600,
    SERIAL_BAUDS_1200,
    SERIAL_BAUDS_2400,
    SERIAL_BAUDS_4800,
    SERIAL_BAUDS_9600,
    SERIAL_BAUDS_19200,
    SERIAL_BAUDS_38400,
    SERIAL_BAUDS_57600,
    SERIAL_BAUDS_115200
} serial_speed_t;


int serial_list_devices(serial_info_t *dev_info, int cap);
void serial_free_list(serial_info_t *dev_info, int num);

serial_t *serial_open(const char *address, serial_speed_t speed);
void serial_close(serial_t *serial);

int serial_read(serial_t *serial, char *buffer, int cap);
int serial_write(serial_t *serial, const char *buffer, int num);

#ifdef __cplusplus
}
#endif

#endif /* ifndef _UART_H_ */
