#ifndef ALPHARAD_ALPHARAD_H
#define ALPHARAD_ALPHARAD_H

#include <stdbool.h>
#include <stdint.h>

void process_image(const uint8_t *p, uint size);

float fps_rolling_average(void);

/* Size of a circular buffer used to measure FPS. Must be a multiple of 8 */
#define FPS_BUFFER_SIZE 16

#endif //ALPHARAD_ALPHARAD_H