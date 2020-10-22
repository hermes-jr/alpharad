#ifndef ALPHARAD_DTO_H
#define ALPHARAD_DTO_H

#include <stdint.h>

typedef struct {
    uint len;
    uint8_t *arr;
} bytes_spawned;

typedef struct {
    uint len;
    uint *arr;
} points_detected;

#endif //ALPHARAD_DTO_H