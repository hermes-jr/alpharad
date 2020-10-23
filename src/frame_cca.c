#include "frame_cca.h"
#include "settings.h"
#include <stdio.h>

extern struct settings settings;

/**
 * Process frame and detect if there are any bright spots in it.
 * Reduce big spots to a single coordinate.
 * A representative is chosen by the means of round robin algorithm to provide some uniformity
 *
 * @param p frame raw data buffer
 * @param size  frame buffer length
 * @param stop_on_first will return as soon as first flash detected
 * @return linked list of representatives of each group of connected pixels
 */
points_detected get_all_flashes(const uint8_t *p, uint size, bool stop_on_first) {
    points_detected result = {0, NULL};

    const uint dw = settings.width * 2;

    for (uint idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            /* We're on black, skip */
            continue;
        }

        uint x = (idx % dw) / 2;
        uint y = idx / dw;

        /* Skip borders, they behave weirdly in my particular camera */
        if (x == 0 || y == 0 || x + 1 == settings.width || y + 1 == settings.height) {
            continue;
        }

        if (stop_on_first) {
            result.len = 1;
            result.arr = malloc(sizeof(idx));
            result.arr[0] = idx;
            return result;
        }

        /*
         * Check neighbors to avoid counting the same particle multiple times.
         * Here should be a proper 'connected components' algorithm implementation
         * but I decided to confine on checking only the closest neighbors.
         * Only a few flashes have radius more than 2.
         * Might be an issue on higher resolutions though, so this is not final.
         */
        // FIXME: one component at a time CCL algorithm should be used here
        // FIXME: temporarily disabled, registering everything. Waiting for alpharadcamerastage2-10 to be resolved
//        if (is_pixel_lit(p, idx - dw) || is_pixel_lit(p, idx - dw - 2) || is_pixel_lit(p, idx - dw + 2) ||
//            is_pixel_lit(p, idx - 2)) {
//            continue;
//        }

    }
    return result;
}

/**
 * Convenience only wrapper method
 * @param p
 * @param size
 * @return true if any pixel is lit
 */
bool has_flashes(const uint8_t *p, uint size) {
    points_detected result = get_all_flashes(p, size, true);
    if (result.len > 0) {
        free(result.arr);
        return true;
    } else {
        return false;
    }
}

/* Return true if requested byte value is greater than settings.threshold */
bool is_pixel_lit(const uint8_t *p, uint idx) { return p[idx] > settings.threshold; }
