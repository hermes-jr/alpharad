#ifndef ALPHARAD_DATA_EXTRACTORS_H
#define ALPHARAD_DATA_EXTRACTORS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "dto.h"

bytes_spawned process_image_default(const uint8_t *p, u_int size);

bytes_spawned process_image_comparator(const uint8_t *p, u_int size);

#if HAVE_OPENSSL

bytes_spawned process_image_sha256_non_blank_frames(const uint8_t *p, u_int size);

bytes_spawned process_image_sha256_all_frames(const uint8_t *p, u_int size);

#endif //HAVE_OPENSSL

bool bit_accumulator(bool bit, uint8_t *ret);

void print_buf_byte_state(ushort);

#endif //ALPHARAD_DATA_EXTRACTORS_H
