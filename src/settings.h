#ifndef ALPHARAD_SETTINGS_H
#define ALPHARAD_SETTINGS_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

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

struct settings {
    uint width;
    uint height;
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

void print_usage(char *self_name);

#endif //ALPHARAD_SETTINGS_H
