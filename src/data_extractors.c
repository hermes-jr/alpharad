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

    points_detected points = get_all_flashes(p, size, FULL_SCAN);

    if (points.len == 0) {
        return result;
    }

    for (uint i = 0; i < points.len; i++) {
        coordinate cp = points.arr[i];

        uint as_idx = settings.width * cp.y + cp.x;
        uint8_t coord_as_byte = round(
                ((double) as_idx / (settings.width * settings.height - 1)) * UINT8_MAX);

        result.len++;
        result.arr = realloc(result.arr, result.len);
        result.arr[result.len - 1] = coord_as_byte;

        bytes++;
        D(log_p(LOG_TRACE, "Flash at %3d:%3d (%d) generated: %3d\n", cp.x, cp.y, as_idx, coord_as_byte));
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

#ifdef DEBUG
    print_buf_byte_state(buf_byte_counter);
#endif //DEBUG

    if (buf_byte_counter++ == 7u) {
        *ret = buf_byte;
        bytes++;
        buf_byte_counter = 0u;
        return true;
    }
    return false;
}

void print_buf_byte_state(ushort buf_byte_counter) {
    if (settings.verbose < LOG_DEBUG) {
        return;
    }
    printf("buf_byte[%d] changed: ", buf_byte_counter);
    for (uint i = 1u << (sizeof(buf_byte) * CHAR_BIT - 1u); i > 0; i = i / 2) {
        (buf_byte & i) ? printf("1") : printf("0");
    }
    printf("\n");
}

/* Fill array of available data extractors */
void register_processors(void) {
    /*
     * Important!
     * Default processor should always be first (this code relies on it).
     * The last one should always be all-NULLs, otherwise we will run into endless loop.
     */
    static frame_processor_t processors[] = {
            {"DEFAULT",
             "Produces normally distributed values",
             process_image_default},

#if HAVE_OPENSSL
            {"SHA256",
             "Calculate SHA256 hash only from frames with detected flashes. Better than SHA256_ALL."
             "Uniform distribution",
             process_image_sha256_non_blank_frames},

            {"SHA256_ALL",
             "Hash is calculated from each frame, which will be mostly black. Thermal noise gives some entropy. "
             "Also uniform distribution. Fastest method (32 bytes per frame)",
             process_image_sha256_all_frames},
#endif //HAVE_OPENSSL

            {"COMPARATOR",
             "Compares two independent values from two consequent frames (with detection) "
             "and spawns a bit. This is the slowest method so far, "
             "it yields approximately 1 byte per 8 flashes",
             process_image_comparator},

            FRAME_PROCESSOR_NULL
    };
    registered_processors = processors;
}