#ifndef ALPHARAD_ALPHARAD_H
#define ALPHARAD_ALPHARAD_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#ifdef DEBUG
#  define D(x) (x)
#else
#  define D(x) do{}while(0)
#endif //DEBUG

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
    enum frame_processor frame_processor;
    char *dev_name;
    char *file_hitlog_name;
    char *file_out_name;
};

bool is_pixel_lit(const u_int8_t *p, u_int idx);

void process_image_default(const u_int8_t *p, u_int size);

void process_image_comparator(const u_int8_t *p, u_int size);

#if HAVE_OPENSSL

void process_image_sha512_non_blank_frames(const u_int8_t *p, u_int size);

void process_image_sha512_all_frames(const u_int8_t *p, u_int size);

#endif //HAVE_OPENSSL

void bit_accumulator(u_int x, u_int y);

void print_buf_byte_state(void);

#endif //ALPHARAD_ALPHARAD_H