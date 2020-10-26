#include <math.h>
#include <limits.h>
#include "data_extractors.h"
#include "settings.h"
#include "frame_cca.h"

#if HAVE_OPENSSL

#include <openssl/sha.h>

#endif //HAVE_OPENSSL

#include <stdio.h>

static uint8_t buf_byte;
ulong bytes = 0;

extern struct settings settings;

bytes_spawned process_image_default(const uint8_t *p, uint size) {
    bytes_spawned result = {0, NULL};

    points_detected points = get_all_flashes(p, size, false);

    if (points.len == 0) {
        return result;
    }

    for (uint i = 0; i < points.len; i++) {
        uint cp = points.arr[i];

        uint8_t coord_as_byte = round(
                ((double) cp / (size - settings.width * 2 - settings.height * 2 - 1)) * UINT8_MAX);

        result.len++;
        result.arr = realloc(result.arr, result.len);
        result.arr[result.len - 1] = coord_as_byte;

        bytes++;
        D(printf("Flash at %3d:%3d (%d) generated: %3d\n", (cp % settings.width * 2) / 2, cp / (settings.width * 2), cp,
                 coord_as_byte));

//        log_flash_at_coordinates(x, y);
    }

    free(points.arr);
    return result;
}

bytes_spawned process_image_comparator(const uint8_t *p, uint size) {
    // FIXME: implement
    (void) size;
    (void) p[0];
    bytes_spawned result = {0, NULL};
    return result;
}

#if HAVE_OPENSSL

bytes_spawned process_image_sha256_all_frames(const uint8_t *p, uint size) {
    bytes_spawned result = {0, NULL};

    result.len = SHA256_DIGEST_LENGTH;
    bytes += result.len;
    result.arr = malloc(result.len * sizeof(result.arr));
    SHA256(p, size, result.arr);

    return result;
}

bytes_spawned process_image_sha256_non_blank_frames(const uint8_t *p, uint size) {
    bytes_spawned result = {0, NULL};

    bool flashes_in_single = has_flashes(p, size);
    if (!flashes_in_single) {
        return result;
    }

    result.len = SHA256_DIGEST_LENGTH;
    bytes += result.len;
    result.arr = malloc(result.len * sizeof(result.arr));
    SHA256(p, size, result.arr);

    return result;
}

#endif //HAVE_OPENSSL

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
