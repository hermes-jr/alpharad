#ifndef ALPHARAD_DTO_H
#define ALPHARAD_DTO_H

#include <stdint.h>

/* Don't forget to free *arr after use! */
typedef struct {
    uint len;
    uint8_t *arr;
} bytes_spawned;

/* Don't forget to free *arr after use! */
typedef struct {
    uint len;
    uint *arr;
} points_detected;

#endif //ALPHARAD_DTO_H