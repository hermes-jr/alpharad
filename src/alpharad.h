#ifndef ALPHARAD_ALPHARAD_H
#define ALPHARAD_ALPHARAD_H

/* Size of a circular buffer used to measure FPS. Must be a multiple of 8 */
#define FPS_BUFFER_SIZE 16

void process_image(const uint8_t *p, uint size);

float fps_rolling_average(void);

#endif //ALPHARAD_ALPHARAD_H