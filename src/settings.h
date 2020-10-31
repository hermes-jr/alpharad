#ifndef ALPHARAD_SETTINGS_H
#define ALPHARAD_SETTINGS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "logger.h"

#ifdef DEBUG
#  define D(x) (x)
#else
#  define D(x) do{}while(0)
#endif //DEBUG

enum frame_processor {
    PROC_DEFAULT,
    PROC_COMPARATOR,
#if HAVE_OPENSSL
    PROC_SHA256_NON_BLANK_FRAMES_ONLY,
    PROC_SHA256_ALL_FRAMES,
#endif //HAVE_OPENSSL
};

#define S_DEFAULT_DEV_NAME "/dev/video0"
#define S_DEFAULT_FILE_OUT_NAME "out.dat"
#define S_DEFAULT_FILE_OUT NULL
#define S_DEFAULT_FILE_HITS_NAME "points.log"
#define S_DEFAULT_FILE_HITS NULL
#define S_DEFAULT_FRAME_PROCESSOR PROC_DEFAULT
#define S_DEFAULT_WIDTH 640
#define S_DEFAULT_HEIGHT 480
#define S_DEFAULT_CROP 0
#define S_DEFAULT_THRESHOLD 8u
#define S_DEFAULT_VERBOSE LOG_FATAL

struct settings {
    uint width;
    uint height;
    uint crop;
    uint8_t threshold;
    uint8_t verbose;
    enum frame_processor frame_processor;
    char *dev_name;
    char *file_hits_name;
    char *file_out_name;
    FILE *file_hits;
    FILE *file_out;
};

int populate_settings(int argc, char **argv, FILE *ofp);

void print_usage(FILE *ofp, char *self_name);

int validated_long_parse(FILE *ofp, uint *target, char *input, const char *err_msg);

#endif //ALPHARAD_SETTINGS_H
