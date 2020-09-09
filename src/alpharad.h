#ifndef ALPHARAD_ALPHARAD_H
#define ALPHARAD_ALPHARAD_H

#include <stdbool.h>
#include <sys/types.h>

#ifdef DEBUG
#  define D(x) (x)
#else
#  define D(x) do{}while(0)
#endif //DEBUG

bool is_pixel_lit(const u_int8_t *p, u_int idx);

void process_image_default(const u_int8_t *p, u_int size);

void process_image_comparator(const u_int8_t *p, u_int size);

#if HAVE_OPENSSL

void process_image_sha512_non_blank_frames(const u_int8_t *p, u_int size);

void process_image_sha512_all_frames(const u_int8_t *p, u_int size);

#endif //HAVE_OPENSSL

void bit_accumulator(u_int x, u_int y);

void print_buf_byte_state(void);

float fps_rolling_average();

#define FPS_BUFFER_SIZE 16 // must be a multiple of 8

#endif //ALPHARAD_ALPHARAD_H