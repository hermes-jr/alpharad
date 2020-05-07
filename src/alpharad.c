#include <signal.h>
#include "v4l2_util.h"

#if HAVE_OPENSSL

#include <openssl/sha.h>

#endif

#include "alpharad.h"
#include <math.h>

struct settings settings = {
        .dev_name = "/dev/video0",
        .file_out_name = "myout.dat",
        .file_hitlog_name = "qa/points.txt",
        .frame_processor = PROC_DEFAULT,
        .width = 640,
        .height = 480
};

int device = -1;
static u_long frame_number = 0;
static u_long bytes = 0;
static struct timeval start, checkpoint;

FILE *out_file;
FILE *stat_file;

static void push_xy(u_int x, u_int y) {
    char z[32];
    sprintf(z, "%d:%d\n", x, y);
    fputs(z, stat_file);
    fflush(stat_file);

//    printf("%d of %d; conv: %d; bytes so far: %07lu\n", idx, size, conv, bytes);
}

static void push_byte(uint8_t conv) {
    fputc(conv, out_file);
    fflush(out_file);
}

bool is_pixel_lit(const u_int8_t *p, u_int idx) { return p[idx] > 32u; }

void process_image(const u_int8_t *p, u_int size) {
    switch (settings.frame_processor) {
#if HAVE_OPENSSL
        case PROC_SHA512_NON_BLANK_FRAMES_ONLY:
            process_image_sha512_non_blank_frames(p, size);
            break;
        case PROC_SHA512_ALL_FRAMES:
            process_image_sha512_all_frames(p, size);
            break;
#endif
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
        /*
         * Check neighbors to avoid counting the same particle multiple times.
         * Here should be a proper 'connected components' algorithm implementation
         * but I decided to confine on checking only the closest neighbors.
         * Only a few flashes have radius more than 2.
         * Might be an issue on higher resolutions though, so this is not final.
         */
        if (idx >= dwidth) {
            if (is_pixel_lit(p, idx - dwidth)) {
                // This is not the first row and N is on
                continue;
            }
            if (idx % dwidth != 0 && is_pixel_lit(p, idx - dwidth - 2)) {
                // Not the leftmost column and NW is on
                continue;
            }
            if ((idx + 2) % dwidth != 0 && is_pixel_lit(p, idx - dwidth + 2)) {
                // Not the rightmost column and NE is on
                continue;
            }
        }
        if (idx % dwidth != 0 && is_pixel_lit(p, idx - 2)) {
            // Not the leftmost column, W is on
            continue;
        }

        // Finally, we found a bright pixel, convert its coordinates to 2 random bytes
        bytes += 2;
        u_int x = (idx % dwidth) / 2;
        uint8_t coord_as_byte1 = roundf(UINT8_MAX * ((double) x / (settings.width - 1)));
        push_byte(coord_as_byte1);

        u_int y = idx / dwidth;
        uint8_t coord_as_byte2 = round(UINT8_MAX * ((double) y / (settings.height - 1)));
        push_byte(coord_as_byte2);
        printf("Flash at %3d:%3d, generated: %3d, %3d\n", x, y, coord_as_byte1, coord_as_byte2);
        push_xy(x, y);
    }
}

#if HAVE_OPENSSL

void process_image_sha512_all_frames(const u_int8_t *p, u_int size) {
    u_char hash[SHA512_DIGEST_LENGTH];
    SHA512(p, size, hash);
    bytes += SHA512_DIGEST_LENGTH;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
        fputc(hash[i], out_file);
    }
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
        for (int i = 0; i < SHA512_DIGEST_LENGTH; i++) {
            fputc(hash[i], out_file);
        }
    }
}

#endif


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

