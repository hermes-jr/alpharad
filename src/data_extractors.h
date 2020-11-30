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

#ifndef ALPHARAD_DATA_EXTRACTORS_H
#define ALPHARAD_DATA_EXTRACTORS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include "dto.h"

bytes_spawned process_image_parity(const uint8_t *p, uint size);

bytes_spawned process_image_rough(const uint8_t *p, uint size);

bytes_spawned process_image_comparator(const uint8_t *p, uint size);

#if HAVE_OPENSSL

bytes_spawned process_image_sha256_non_blank_frames(const uint8_t *p, uint size);

bytes_spawned process_image_sha256_all_frames(const uint8_t *p, uint size);

#endif //HAVE_OPENSSL

bool bit_accumulator(bool bit, uint8_t *ret);

void print_buf_byte_state(ushort);

void register_processors(void);

#endif //ALPHARAD_DATA_EXTRACTORS_H
