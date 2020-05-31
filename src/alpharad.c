#include <signal.h>
#include "v4l2_util.h"

#if HAVE_OPENSSL

#include <openssl/sha.h>

#endif //HAVE_OPENSSL

#include "alpharad.h"
#include <math.h>
#include <limits.h>

struct settings settings = {
        .dev_name = "/dev/video0",
        .file_out_name = "out.dat",
        .file_hitlog_name = "points.log",
        .frame_processor = PROC_COMPARATOR,
//        .frame_processor = PROC_DEFAULT,
        .width = 640,
        .height = 480,
        .threshold = 8u
};

int device = -1;
static u_long frame_number = 0;
static u_long bytes = 0;
static struct timeval start, checkpoint;

uint8_t buf_byte;
uint8_t buf_byte_counter;

FILE *out_file;
FILE *stat_file;

static void log_flash_at_coordinates(u_int x, u_int y) {
    char z[32];
    snprintf(z, 32, "%d:%d\n", x, y);
    fputs(z, stat_file);
    fflush(stat_file);
}

static void spawn_byte(uint8_t conv) {
    D(printf("spawning %d\n", conv));
    fputc(conv, out_file);
    fflush(out_file);
}

bool is_pixel_lit(const u_int8_t *p, u_int idx) { return p[idx] > settings.threshold; }

void process_image(const u_int8_t *p, u_int size) {
    switch (settings.frame_processor) {
#if HAVE_OPENSSL
        case PROC_SHA512_NON_BLANK_FRAMES_ONLY:
            process_image_sha512_non_blank_frames(p, size);
            break;
        case PROC_SHA512_ALL_FRAMES:
            process_image_sha512_all_frames(p, size);
            break;
#endif //HAVE_OPENSSL
        case PROC_COMPARATOR:
            process_image_comparator(p, size);
            break;
        case PROC_DEFAULT:
        default:
            process_image_default(p, size);
            break;
    }
}

void process_image_default(const u_int8_t *p, u_int size) {
    const u_int dwidth = settings.width * 2;

    for (u_int idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            // We're on black, skip to next pixel
            continue;
        }

        u_int x = (idx % dwidth) / 2;
        u_int y = idx / dwidth;

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
        if (is_pixel_lit(p, idx - dwidth) || is_pixel_lit(p, idx - dwidth - 2) || is_pixel_lit(p, idx - dwidth + 2) ||
            is_pixel_lit(p, idx - 2)) {
            continue;
        }

/*
        // Finally, we found a bright pixel, convert its coordinates to 2 random bytes
        bytes += 2;
        uint8_t coord_as_byte1 = round(((double) (x - 1) / (settings.width - 2)) * UINT8_MAX);
        spawn_byte(coord_as_byte1);

        uint8_t coord_as_byte2 = round(((double) (y - 1) / (settings.height - 2)) * UINT8_MAX);
        spawn_byte(coord_as_byte2);
        D(printf("Flash at %3d:%3d, generated: %3d, %3d\n", x, y, coord_as_byte1, coord_as_byte2));
*/
        bytes++;
        D(printf("Flash at %3d:%3d; %d / %d\n", x, y, idx, size));
        uint8_t coord_as_byte = round(
                ((double) idx / (size - settings.width * 2 - settings.height * 2 - 1)) * UINT8_MAX);
        spawn_byte(coord_as_byte);
        D(printf("Flash at %3d:%3d, generated: %3d\n", x, y, coord_as_byte));

        log_flash_at_coordinates(x, y);
    }
}

void process_image_comparator(const u_int8_t *p, u_int size) {
    const u_int dwidth = settings.width * 2;

    for (u_int idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            // We're on black, skip to next pixel
            continue;
        }

        u_int x = (idx % dwidth) / 2;
        u_int y = idx / dwidth;

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
        if (is_pixel_lit(p, idx - dwidth) || is_pixel_lit(p, idx - dwidth - 2) || is_pixel_lit(p, idx - dwidth + 2) ||
            is_pixel_lit(p, idx - 2)) {
            continue;
        }

        bit_accumulator(x, y);

        log_flash_at_coordinates(x, y);
    }
}

void bit_accumulator(u_int x, u_int y) {
    double v1 = (double) (x - 1) / (settings.width - 2);
    double v2 = (double) (y - 1) / (settings.height - 2);
    if (v1 == v2) {
        D(printf("Equal doubles detected, skipping"));
        return;
    }

    buf_byte <<= 1u;
    if (v1 < v2) {
        buf_byte |= 1u;
    }

    D(print_buf_byte_state());

    if (buf_byte_counter++ == sizeof(buf_byte) * CHAR_BIT - 1u) {
        bytes++;
        spawn_byte(buf_byte);
//        buf_byte = 0u;
        buf_byte_counter = 0u;
    }
}

void print_buf_byte_state(void) {
    printf("buf_byte[%d] changed: ", buf_byte_counter);
    for (uint i = 1u << (sizeof(buf_byte) * CHAR_BIT - 1u); i > 0; i = i / 2) {
        (buf_byte & i) ? printf("1") : printf("0");
    }
    printf("\n");
}

#if HAVE_OPENSSL

void process_image_sha512_all_frames(const u_int8_t *p, u_int size) {
    u_char hash[SHA512_DIGEST_LENGTH];
    SHA512(p, size, hash);
    bytes += SHA512_DIGEST_LENGTH;
    fwrite(hash, 1, SHA512_DIGEST_LENGTH, out_file);
    fflush(out_file);
}

void process_image_sha512_non_blank_frames(const u_int8_t *p, u_int size) {
    for (u_int idx = 0; idx < size; idx += 2) {
        if (!is_pixel_lit(p, idx)) {
            // We're on black, skip to next pixel
            continue;
        }
        u_char hash[SHA512_DIGEST_LENGTH];
        SHA512(p, size, hash);
        bytes += SHA512_DIGEST_LENGTH;
        fwrite(hash, 1, SHA512_DIGEST_LENGTH, out_file);
        fflush(out_file);
    }
}

#endif //HAVE_OPENSSL


static void print_perf_stats(void) {
    gettimeofday(&checkpoint, NULL);
    double time_spent = (double) (checkpoint.tv_usec - start.tv_usec) / 1.0e6 +
                        (double) (checkpoint.tv_sec - start.tv_sec);
    printf("Time: %d, FPS: %f, BPM: %f\n",
           (int) time_spent,
           ((double) frame_number) / time_spent,
           ((double) bytes) / (time_spent / 60));
}

static void main_loop(void) {

    for (;;) {
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(device, &fds);

        /* Timeout. */
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        r = select(device + 1, &fds, NULL, NULL, &tv);

        if (-1 == r) {
            if (EINTR == errno)
                continue;
            errno_exit("select");
        }

        if (0 == r) {
            fprintf(stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }

        if (!read_frame(&process_image)) {
            fprintf(stderr, "recording error\n");
            break;
        }
    }
}

static void signal_usr1_handler(int signum) {
    if (SIGUSR1 == signum) {
        print_perf_stats();
    }
}

int main(int argc, char **argv) {
    // get rid of unused warning
    (void) argv;
    (void) argc;

    // Avoid division by zero
    assert(settings.width > 2);
    assert(settings.height > 2);

    signal(SIGUSR1, signal_usr1_handler);

    out_file = fopen(settings.file_out_name, "ab");
    stat_file = fopen(settings.file_hitlog_name, "a");

    gettimeofday(&start, NULL);

    open_device();
    init_device();
    start_capturing();
    main_loop();
    stop_capturing();
    uninit_device();
    close_device();

    fclose(out_file);

    fprintf(stderr, "\n");
    return EXIT_SUCCESS;
}

