/*
Copyright (C) 2020 Mikhail Antonov <hermes@cyllene.net>

This file is part of alpharad project.

alpharad is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

alpharad is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with alpharad.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* define CLEAR(x) */
#include <errno.h>
#include <assert.h>
#include <sys/time.h>
#include "settings.h"
#include "data_extractors.h"
#include "v4l2_util.h"
#include "alpharad.h"

extern struct settings settings;
extern ulong bytes;

int device = -1;
static struct timeval start_time;
static volatile bool exit_requested = false;

uint *fps_buffer;
uint8_t fps_buffer_idx = 0;

void process_image(const uint8_t *p, uint size) {
    bytes_spawned bs = settings.frame_processor.execute(p, size);

    if (bs.len > 0) {
        if (settings.file_out != NULL) {
            fwrite(bs.arr, 1, bs.len, settings.file_out);
            fflush(settings.file_out);
        }
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
float fps_rolling_average(void) {
    float sum = 0u;
    D(log_p(LOG_DEBUG, "Frame durations rolling buffer: "));
    for (int j = 0; j < FPS_BUFFER_SIZE; j++) {
        D(log_p(LOG_DEBUG, "%d, ", fps_buffer[j]));
        sum += fps_buffer[j];
    }
    float avg_frame_time = sum / FPS_BUFFER_SIZE;
    D(log_p(LOG_DEBUG, "\nAVG frame time: %fns\n", avg_frame_time));
    return 1.0e6f / avg_frame_time;
}

static void main_loop(void) {

    struct timeval frame_start, frame_end, frame_duration;

    for (; !exit_requested;) {
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
            log_fp(LOG_FATAL, stderr, "select timeout\n");
            exit(EXIT_FAILURE);
        }

        if (!read_frame(&process_image)) {
            log_fp(LOG_FATAL, stderr, "recording error\n");
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

static void signal_sigint_handler(int signum) {
    if (SIGINT == signum) {
        exit_requested = true;
    }
}

static void pre_exit_cleanup(void) {
    uninit_device();
    close_device();

    if (settings.file_out != NULL) {
        fclose(settings.file_out);
    }
    if (settings.file_hits != NULL) {
        fclose(settings.file_hits);
    }
    free(fps_buffer);
}

int main(int argc, char **argv) {
    /* Get rid of unused warning */
    (void) argv;
    (void) argc;

    register_processors();

    int settings_ret = populate_settings(stdout, argv, argc);
    if (settings_ret == -1) {
        exit(EXIT_FAILURE);
    } else if (settings_ret == 1) {
        exit(EXIT_SUCCESS);
    }
    /* Avoid division by zero */
    assert(settings.width > 2);
    assert(settings.height > 2);

    if (is_processor_null(&settings.frame_processor)) {
        settings.frame_processor = registered_processors[0];
    }

    /* FPS and other stats initialization */
    log_p(LOG_INFO, "Initializing FPS buffer\n");
    gettimeofday(&start_time, NULL);
    fps_buffer = calloc(FPS_BUFFER_SIZE, sizeof(fps_buffer));
    signal(SIGUSR1, signal_usr1_handler);
    signal(SIGINT, signal_sigint_handler);

    settings.file_out = fopen(settings.file_out_name, "ab");
    settings.file_hits = fopen(settings.file_hits_name, "a");

    log_p(LOG_INFO, "Opening video device\n");
    open_device();
    log_p(LOG_INFO, "Initializing video device\n");
    init_device();
    log_p(LOG_INFO, "Start capturing\n");
    start_capturing();
    main_loop();
    log_p(LOG_INFO, "Main loop is over, stopping\n");
    stop_capturing();

    log_p(LOG_INFO, "Cleaning up\n");
    pre_exit_cleanup();

    log_p(LOG_INFO, "Bye\n");
    return EXIT_SUCCESS;
}

