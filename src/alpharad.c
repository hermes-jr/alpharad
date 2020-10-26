#include <signal.h>
#include "alpharad.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* define CLEAR(x) */
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include "settings.h"
#include "data_extractors.h"
#include "v4l2_util.h"

extern struct settings settings;
extern ulong bytes;

int device = -1;
static struct timeval start_time;

uint *fps_buffer;
uint8_t fps_buffer_idx = 0;

FILE *out_file;
FILE *stat_file;

/*
static void log_flash_at_coordinates(uint x, uint y) {
    char z[32];
    snprintf(z, 32, "%d:%d\n", x, y);
    fputs(z, stat_file);
    fflush(stat_file);
}
*/

void process_image(const uint8_t *p, uint size) {
    bytes_spawned bs;
    switch (settings.frame_processor) {
#if HAVE_OPENSSL
        case PROC_SHA256_NON_BLANK_FRAMES_ONLY:
            bs = process_image_sha256_non_blank_frames(p, size);
            break;
        case PROC_SHA256_ALL_FRAMES:
            bs = process_image_sha256_all_frames(p, size);
            break;
#endif //HAVE_OPENSSL
        case PROC_COMPARATOR:
            bs = process_image_comparator(p, size);
            break;
        case PROC_DEFAULT:
        default:
            bs = process_image_default(p, size);
            break;
    }

    if (bs.len > 0) {
        fwrite(bs.arr, 1, bs.len, out_file);
        fflush(out_file);
        free(bs.arr);
    }

}

static void print_perf_stats(void) {
    struct timeval bpm_checkpoint, time_diff;
    gettimeofday(&bpm_checkpoint, NULL);
    timersub(&bpm_checkpoint, &start_time, &time_diff);
    float time_spent = (float) time_diff.tv_sec + (float) time_diff.tv_usec / 1.0e6f;
    printf("Time: %d, FPS: %f, bytes: %lu, BPM: %f\n",
           (int) time_spent,
           fps_rolling_average(),
           bytes,
           ((double) bytes) / (time_spent / 60));
}

/* Get average frame processing time from a circular buffer, estimate Frames Per Second */
float fps_rolling_average() {
    float sum = 0u;
    D(printf("Frame durations rolling buffer: "));
    for (int j = 0; j < FPS_BUFFER_SIZE; j++) {
        D(printf("%d, ", fps_buffer[j]));
        sum += fps_buffer[j];
    }
    float avg_frame_time = sum / FPS_BUFFER_SIZE;
    D(printf("\nAVG frame time: %fns\n", avg_frame_time));
    return 1.0e6f / avg_frame_time;
}

static void main_loop(void) {

    struct timeval frame_start, frame_end, frame_duration;

    for (;;) {
        gettimeofday(&frame_start, NULL);
        fd_set fds;
        struct timeval tv;
        int r;

        FD_ZERO(&fds);
        FD_SET(device, &fds); // NOLINT(hicpp-signed-bitwise)

        /* Timeout */
        tv.tv_sec = 0;
        tv.tv_usec = 700000;

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

        gettimeofday(&frame_end, NULL);
        timersub(&frame_end, &frame_start, &frame_duration);
        fps_buffer[fps_buffer_idx++ % FPS_BUFFER_SIZE] = frame_duration.tv_sec * 1e6 + frame_duration.tv_usec;
    }
}

static void signal_usr1_handler(int signum) {
    if (SIGUSR1 == signum) {
        print_perf_stats();
    }
}

int main(int argc, char **argv) {
    /* Get rid of unused warning */
    (void) argv;
    (void) argc;

    int settings_ret = populate_settings(argc, argv, stdout);
    if (settings_ret == -1) {
        exit(EXIT_FAILURE);
    } else if (settings_ret == 1) {
        exit(EXIT_SUCCESS);
    }
    /* Avoid division by zero */
    assert(settings.width > 2);
    assert(settings.height > 2);

    /* FPS and other stats initialization */
    gettimeofday(&start_time, NULL);
    fps_buffer = (uint *) calloc(FPS_BUFFER_SIZE, sizeof(fps_buffer[0]));
    signal(SIGUSR1, signal_usr1_handler);

    out_file = fopen(settings.file_out_name, "ab");
    stat_file = fopen(settings.file_hits_name, "a");

    open_device();
    init_device();
    start_capturing();
    main_loop();
    stop_capturing();
    uninit_device();
    close_device();

    fclose(out_file);
    free(fps_buffer);

    fprintf(stderr, "\n");
    return EXIT_SUCCESS;
}

