#ifndef ALPHARAD_ALPHARAD_H
#define ALPHARAD_ALPHARAD_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

enum frame_processor {
    PROC_DEFAULT,
#if HAVE_OPENSSL
    PROC_SHA512_NON_BLANK_FRAMES_ONLY,
    PROC_SHA512_ALL_FRAMES,
#endif
};

struct settings {
    char *dev_name;
    char *file_hitlog_name;
    char *file_out_name;
    enum frame_processor frame_processor;
    int width;
    int height;
};

bool is_pixel_lit(const u_int8_t *p, u_int idx);

void process_image_default(const u_int8_t *p, u_int size);

#if HAVE_OPENSSL

void process_image_sha512_non_blank_frames(const u_int8_t *p, u_int size);

void process_image_sha512_all_frames(const u_int8_t *p, u_int size);

#endif

#endif //ALPHARAD_ALPHARAD_H