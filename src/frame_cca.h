#ifndef ALPHARAD_FRAME_CCA_H
#define ALPHARAD_FRAME_CCA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dto.h"

bool is_pixel_lit(const uint8_t *p, uint idx);

/**
 * Process frame and detect if there are any
 *
 * @param p frame raw data buffer
 * @param size  frame buffer length
 * @param stop_on_first will return as soon as first flash detected
 * @return linked list of representatives of each group of connected pixels
 */
points_detected get_all_flashes(const uint8_t *p, u_int size, bool stop_on_first);

/**
 * Convenience only wrapper method
 * @param p
 * @param size
 * @return true if any pixel is lit
 */
bool has_flashes(const uint8_t *p, u_int size);

#endif //ALPHARAD_FRAME_CCA_H
