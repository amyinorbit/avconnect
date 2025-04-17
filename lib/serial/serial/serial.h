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

typedef struct serial_t serial_t;

typedef struct {
    const char *address;
    const char *name;
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

serial_t *serial_open(const char *address, serial_speed_t speed);
void serial_close(serial_t *serial);

size_t serial_read(serial_t *serial, char *buffer, size_t cap);
size_t serial_write(serial_t *serial, const char *buffer, size_t num);

#endif /* ifndef _UART_H_ */
