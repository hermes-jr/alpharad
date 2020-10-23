#include <math.h>
#include <limits.h>
#include "data_extractors.h"
#include "settings.h"
#include "frame_cca.h"

#if HAVE_OPENSSL

#include <openssl/sha.h>
#include <stdio.h>

#endif //HAVE_OPENSSL

static uint8_t buf_byte;

extern struct settings settings;

bytes_spawned process_image_default(const uint8_t *p, uint size) {
    bytes_spawned result = {0, NULL};
    const uint dw = settings.width * 2;

    points_detected points = get_all_flashes(p, size, false);

    if (points.len == 0) {
        return result;
    }

    for (uint idx = 0; idx < points.len; idx++) {
        uint cp = points.arr[idx];

        uint x = (cp % dw) / 2;
        uint y = cp / dw;

        if (x == 0 || y == 0 || x + 1 == settings.width || y + 1 == settings.height) {
            // Skip borders, they behave weirdly
            continue;
        }

        /*
         * Check neighbors to avoid counting the same particle multiple times.
         * Here should be a proper 'connected components' algorithm implementation
         * but I decided to confine on checking only the closest neighbors.
         * Only a few flashes have radius more than 2.
         * Might be an issue on higher resolutions though, so this is not final.
         */
        // FIXME: temporarily disabled, registering everything. Waiting for alpharadcamerastage2-10 to be resolved
//        if (is_pixel_lit(p, idx - dw) || is_pixel_lit(p, idx - dw - 2) || is_pixel_lit(p, idx - dw + 2) ||
//            is_pixel_lit(p, idx - 2)) {
//            continue;
//        }

        D(printf("Flash at %3d:%3d; %d / %d\n", x, y, idx, size));
        uint8_t coord_as_byte = round(
                ((double) idx / (size - settings.width * 2 - settings.height * 2 - 1)) * UINT8_MAX);
//        spawn_byte(coord_as_byte);
        bytes++;
        D(printf("Flash at %3d:%3d, generated: %3d\n", x, y, coord_as_byte));

//        log_flash_at_coordinates(x, y);
    }
    return result;
}

bytes_spawned process_image_comparator(const uint8_t *p, uint size) {
    const uint dw = settings.width * 2;

    for (uint idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            // We're on black, skip to next pixel
            continue;
        }

        uint x = (idx % dw) / 2;
        uint y = idx / dw;

        if (x == 0 || y == 0 || x + 1 == settings.width || y + 1 == settings.height) {
            // Skip borders, they behave weirdly
            continue;
        }

        /*
         * Check neighbors to avoid counting the same particle multiple times.
         * Here should be a proper 'connected components' algorithm implementation
         * but I decided to confine on checking only the closest neighbors.
         * Only a few flashes have radius more than 2.
         * Might be an issue on higher resolutions though, so this is not final.
         */
        if (is_pixel_lit(p, idx - dw) || is_pixel_lit(p, idx - dw - 2) || is_pixel_lit(p, idx - dw + 2) ||
            is_pixel_lit(p, idx - 2)) {
            continue;
        }

//        bit_accumulator(x, y);

//        log_flash_at_coordinates(x, y);
    }
    bytes_spawned result = {0, NULL};
    return result;
}

bool bit_accumulator(bool bit, uint8_t *ret) {

    static ushort buf_byte_counter;

    buf_byte <<= 1u;
    if (bit) {
        buf_byte |= 1u;
    }

    D(print_buf_byte_state(buf_byte_counter));

    if (buf_byte_counter++ == 7u) {
        *ret = buf_byte;
        bytes++;
        buf_byte_counter = 0u;
        return true;
    }
    return false;
}

void print_buf_byte_state(ushort buf_byte_counter) {
    printf("buf_byte[%d] changed: ", buf_byte_counter);
    for (uint i = 1u << (sizeof(buf_byte) * CHAR_BIT - 1u); i > 0; i = i / 2) {
        (buf_byte & i) ? printf("1") : printf("0");
    }
    printf("\n");
}

#if HAVE_OPENSSL

bytes_spawned process_image_sha256_all_frames(const uint8_t *p, uint size) {
    u_char hash[SHA256_DIGEST_LENGTH];
    SHA256(p, size, hash);
    bytes += SHA256_DIGEST_LENGTH;
//    fwrite(hash, 1, SHA256_DIGEST_LENGTH, out_file);
//    fflush(out_file);
    bytes_spawned result = {0, NULL};
    return result;
}

bytes_spawned process_image_sha256_non_blank_frames(const uint8_t *p, uint size) {
    for (uint idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            /* We're on black, skip to next pixel */
            continue;
        }
        u_char hash[SHA256_DIGEST_LENGTH];
        SHA256(p, size, hash);
        bytes += SHA256_DIGEST_LENGTH;
//        fwrite(hash, 1, SHA256_DIGEST_LENGTH, out_file);
//        fflush(out_file);
    }
    bytes_spawned result = {0, NULL};
    return result;
}

#endif //HAVE_OPENSSL
