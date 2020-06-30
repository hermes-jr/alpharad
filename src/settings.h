#ifndef ALPHARAD_SETTINGS_H
#define ALPHARAD_SETTINGS_H

#include <stdlib.h>

enum frame_processor {
    PROC_DEFAULT,
    PROC_COMPARATOR,
#if HAVE_OPENSSL
    PROC_SHA512_NON_BLANK_FRAMES_ONLY,
    PROC_SHA512_ALL_FRAMES,
#endif //HAVE_OPENSSL
};

struct settings {
    u_int width;
    u_int height;
    u_int8_t threshold;
    u_int8_t verbose;
    enum frame_processor frame_processor;
    char *dev_name;
    char *file_hitlog_name;
    char *file_out_name;
};

void populate_settings(int argc, char **argv);

void print_usage(char *self_name);

#endif //ALPHARAD_SETTINGS_H
