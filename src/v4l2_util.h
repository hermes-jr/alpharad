#ifndef ALPHARAD_V4L2_UTIL_H
#define ALPHARAD_V4L2_UTIL_H

#include <stdint.h>

#define CLEAR(x) memset(&(x), 0, sizeof(x))

struct buffer {
    uint8_t *start;
    size_t length;
};

int xioctl(int dev, int request, void *arg);

//int read_frame(void);
int read_frame(void (*callback)(const uint8_t *p, uint size));

void stop_capturing(void);

void start_capturing(void);

void init_mmap(void);

void uninit_device(void);

void init_device(void);

void close_device(void);

void open_device(void);

void errno_exit(const char *s);

#endif //ALPHARAD_V4L2_UTIL_H