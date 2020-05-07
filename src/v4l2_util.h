#ifndef ALPHARAD_V4L2_UTIL_H
#define ALPHARAD_V4L2_UTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* define CLEAR(x) */
#include <sys/stat.h>
#include <fcntl.h> /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
    u_int8_t *start;
    size_t length;
};

int xioctl(int dev, int request, void *arg);

//int read_frame(void);
int read_frame(void (*callback)(const u_int8_t *p, u_int size));

void stop_capturing(void);

void start_capturing(void);

void init_mmap(void);

void uninit_device(void);

void init_device(void);

void close_device(void);

void open_device(void);

void errno_exit(const char *s);

#endif //ALPHARAD_V4L2_UTIL_H