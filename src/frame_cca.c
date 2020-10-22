#include "frame_cca.h"
#include "settings.h"

extern struct settings settings;

points_detected get_all_flashes(const u_int8_t *p, u_int size, bool stop_on_first) {
    points_detected result = {0, NULL};

    const u_int dw = settings.width * 2;

    for (u_int idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            /* We're on black, skip */
            continue;
        }

        u_int x = (idx % dw) / 2;
        u_int y = idx / dw;

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

bool is_pixel_lit(const u_int8_t *p, u_int idx) { return p[idx] > settings.threshold; }
