#ifndef ALPHARAD_FRAME_CCA_H
#define ALPHARAD_FRAME_CCA_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "dto.h"

bool is_pixel_lit(const uint8_t *p, uint idx);

points_detected get_all_flashes(const uint8_t *p, uint size, bool stop_on_first);

bool has_flashes(const uint8_t *p, uint size);

#endif //ALPHARAD_FRAME_CCA_H
