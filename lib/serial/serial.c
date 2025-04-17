/*===--------------------------------------------------------------------------------------------===
 * serial.c
 *
 * Created by Amy Parent <amy@amyparent.com>
 * Copyright (c) 2025 Amy Parent. All rights reserved
 *
 * Licensed under the MIT License
 *===--------------------------------------------------------------------------------------------===
*/
#include <serial/serial.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#if defined(__APPLE__) || defined(__linux__)
#define USE_POSIX   (1)
#define USE_WIN32   (0)

#include <sys/types.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>

#else
#define USE_POSIX   (0)
#define USE_WIN32   (1)
#endif



struct serial_t {
#if USE_POSIX
    int fd;
#else
#error "non-posix platforms not (yet) supported"
#endif
};

serial_t *serial_open(const char *address, serial_speed_t speed_bds) {
#if USE_POSIX
    struct termios options = {};
    speed_t speed;
    
    
    int fd = open(address, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd < 0) {
        fprintf(stderr, "error: %s", strerror(errno));
        return NULL;
    }
    fcntl(fd, F_SETFL, FNDELAY);
    tcgetattr(fd, &options);
    memset(&options, 0, sizeof(options));
    
    switch(speed_bds) {
        case SERIAL_BAUDS_300: speed = B300;
        case SERIAL_BAUDS_600: speed = B600;
        case SERIAL_BAUDS_1200: speed = B1200;
        case SERIAL_BAUDS_2400: speed = B2400;
        case SERIAL_BAUDS_4800: speed = B4800;
        case SERIAL_BAUDS_9600: speed = B9600;
        case SERIAL_BAUDS_19200: speed = B19200;
        case SERIAL_BAUDS_38400: speed = B38400;
        case SERIAL_BAUDS_57600: speed = B57600;
        case SERIAL_BAUDS_115200: speed = B115200;
    }
    int databits_flag = CS8; // 8 data bits
    int stopbits_flag = 0; // 1 stop bit
    int parity_flag = 0; // no parity bit
    
    cfsetispeed(&options, speed);
    cfsetospeed(&options, speed);
    
    options.c_cflag |= (CLOCAL | CREAD | databits_flag | parity_flag | stopbits_flag);
    options.c_iflag |= (IGNPAR | IGNBRK);
    options.c_cc[VTIME] = 0;
    options.c_cc[VMIN] = 0;
    tcsetattr(fd, TCSANOW, &options);
    
    serial_t *serial = calloc(1, sizeof(serial));
    if(serial == NULL)
        return NULL;
    serial->fd = fd;
    return serial;
#else
    return NULL;
#endif
}

void serial_close(serial_t *serial) {
#if USE_POSIX
    close(serial->fd);
    serial->fd = -1;
#else
    
#endif
    free(serial);
}

size_t serial_read(serial_t *serial, char *buffer, size_t cap) {
#if USE_POSIX
    return read(serial->fd, buffer, cap);
#else
    return 0;
#endif
}

size_t serial_write(serial_t *serial, const char *buffer, size_t num) {
#if USE_POSIX
    return write(serial->fd, buffer, num);
#else
    return 0;
#endif
}
