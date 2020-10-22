#ifndef ALPHARAD_ALPHARAD_H
#define ALPHARAD_ALPHARAD_H

#include <stdbool.h>
#include <stdint.h>

float fps_rolling_average();

/* Size of a circular buffer used to measure FPS. Must be a multiple of 8 */
#define FPS_BUFFER_SIZE 16

#endif //ALPHARAD_ALPHARAD_H